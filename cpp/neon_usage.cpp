#include <stdio.h>
#include <arm_neon.h> // need to include this if you want to use intrinsics

//
// Building: rm-linux-gnueabi-g++  -mfloat-abi=softfp -mfpu=neon  neon_usage.cpp -o neon_usage
//

int main(){
    // vector addition 8x8 example.
    uint8x8_t vec_a, vec_b, vec_dest;   // a vector of 8 8bit ints
    vec_a = vdup_n_u8(9);
    vec_b = vdup_n_u8(10);
    vec_dest = vec_a * vec_b;           // 90
    int i = 0;
    int result;
    result = vget_lane_u8( vec_dest, 0 );
    printf( "Lane %d: %d\n", i, result );
    i++;
    result = vget_lane_u8( vec_dest, 1 );
    printf( "Lane %d: %d\n", i, result );
    i++;
    result = vget_lane_u8( vec_dest, 2 );
    printf( "Lane %d: %d\n", i, result );
    i++;
    result = vget_lane_u8( vec_dest, 3 );
    printf( "Lane %d: %d\n", i, result );
    i++;
    result = vget_lane_u8( vec_dest, 4 );
    printf( "Lane %d: %d\n", i, result );
    i++;
    result = vget_lane_u8( vec_dest, 5 );
    printf( "Lane %d: %d\n", i, result );
    i++;
    result = vget_lane_u8( vec_dest, 6 );
    printf( "Lane %d: %d\n", i, result );
    i++;
    result = vget_lane_u8( vec_dest, 7 );
    printf( "Lane %d: %d\n", i, result );
}
