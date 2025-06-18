import os
import time
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtCore import QTimer, Qt
from picamera2 import Picamera2
from picamera2.encoders import H264Encoder
from picamera2.outputs import FfmpegOutput
import cv2
import numpy as np

class CameraApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("RPi Digital Camera")
        self.setFixedSize(320, 240)

        # Инициализация камеры
        self.picam2 = Picamera2()
        
        # Конфигурация для фото (RAW + JPEG)
        self.photo_config = self.picam2.create_still_configuration(
            raw={"size": self.picam2.sensor_resolution},
            display="main",
            buffer_count=2
        )
        
        # Конфигурация для видео (1080p24)
        self.video_config = self.picam2.create_video_configuration(
            main={"size": (1920, 1080)},
            controls={"FrameRate": 24}
        )
        self.picam2.configure(self.video_config)
        
        # Кодек H264
        self.encoder = H264Encoder(bitrate=10_000_000)
        self.output = FfmpegOutput("output.mp4", audio=False)
        
        # GUI
        self.central_widget = QWidget()
        self.layout = QVBoxLayout()
        self.video_label = QLabel()
        self.video_label.setAlignment(Qt.AlignCenter)
        self.layout.addWidget(self.video_label)
        self.central_widget.setLayout(self.layout)
        self.setCentralWidget(self.central_widget)
        
        # Таймер для обновления кадров
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_frame)
        self.timer.start(30)  # ~30 FPS
        
        # Флаги
        self.is_recording = False
        self.overlay_text = ""

    def update_frame(self):
        # Получаем кадр в RGB
        frame = self.picam2.capture_array("main")
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
        
        # Наложение оверлея (текст, время, etc.)
        cv2.putText(
            frame_rgb,
            f"{time.strftime('%H:%M:%S')} | {'REC' if self.is_recording else ''}",
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (0, 0, 255),
            1
        )
        
        # Ресайз для экрана 320x240
        frame_small = cv2.resize(frame_rgb, (320, 240))
        
        # Конвертация для PyQt
        h, w, ch = frame_small.shape
        bytes_per_line = ch * w
        q_img = QImage(frame_small.data, w, h, bytes_per_line, QImage.Format_RGB888)
        self.video_label.setPixmap(QPixmap.fromImage(q_img))
        
        # Запись видео (если включено)
        if self.is_recording:
            self.encoder.encode(frame_rgb)

    def start_recording(self):
        self.picam2.switch_mode(self.video_config)
        self.picam2.start_encoder(self.encoder, self.output)
        self.is_recording = True
        self.overlay_text = "REC"

    def stop_recording(self):
        self.picam2.stop_encoder()
        self.is_recording = False
        self.overlay_text = ""

    def capture_raw_photo(self):
        self.picam2.switch_mode(self.photo_config)
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        self.picam2.capture_file(f"photo_{timestamp}.dng", format="dng")
        self.picam2.switch_mode(self.video_config)

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_R:
            if not self.is_recording:
                self.start_recording()
            else:
                self.stop_recording()
        elif event.key() == Qt.Key_Space:
            self.capture_raw_photo()

if __name__ == "__main__":
    app = QApplication([])
    window = CameraApp()
    window.show()
    app.exec_()