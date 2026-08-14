#pragma once

#define READBIT(A, B) ((A >> (B & 7)) & 1)
#define SETBIT(T, B, V) (T = V ? T | (1<<B) : T & ~(1<<B))

//#define COPYBITS(T,S,N) ((T) = ((T) & (~0x0ul << (N))) | ((S) & ~(~0x0ul << (N))))

//#define COPYBIT(T, S, N) ((T) = ((T) & ~(0x1ul << (N))) | ((S) & (0x1ul << (N))))

/* A final refinement */
/* CLEARBIT evaluates to T with the order N bit to 0 */
#define CLEARBIT(T,N) ((T) & ~(0x1ul << (N)))

/* CLEARBITS evaluates to T with the N lowest order bits to 0 */
#define CLEARBITS(T,N) ((T) & (~0x0ul << (N)))

/* GETBIT evaluates to S, keeping only the order N bit */
#define GETBIT(S,N) ((S) & (0x1ul << (N)))

/* GETBITS evaluates to S, keeping only the N lowest order bits */
#define GETBITS(S,N) ((S) & ~(~0x0ul << (N)))

/* COPYBIT copies the order N bit of S to T */
#define COPYBIT(T,S,N) ((T) = ((T) & CLEARBIT((T),(N))) | (GETBIT((S),(N))))

/* COPYBITS copies the N lowest order bits of S to T */
#define COPYBITS(T,S,N) ((T) = ((T) & CLEARBITS((T),(N))) | (GETBITS((S),(N))))

