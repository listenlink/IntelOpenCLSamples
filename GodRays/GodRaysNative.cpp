// Copyright (c) 2009-2011 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#ifndef __linux__
#include "stdafx.h"
#else
#include "emmintrin.h"
#include "math.h"
#endif

#define GOD_RAYS_BUNCH_SIZE 15
#define DECAY -0.01f
#define WEIGHT 3.0f
#define INVEXPOSURE 1.f/.9f
#define FTHRESHOLD 1.8f

void EvaluateRay(
                 float* inBuf,
                 int iw,
                 int ih,
                 int blend,
                 float* outBuf,
                 int in_RayNum,
                 int god_rays_b_size
                 )
{
    int LightScreenPosX = iw/2;
    int LightScreenPosY = ih/2;

    //---------------------------------------------------------
    //-----------God Rays SSE HDR Post Processing here---------
    //---------------------------------------------------------
    in_RayNum *= god_rays_b_size;

    for(int i = in_RayNum; i < in_RayNum+god_rays_b_size; i++ )
    {
        //in_pData params
        float _decay = (float)DECAY;
        float _Weight = (float)WEIGHT;
        float _InvExposure = (float)INVEXPOSURE;
        float _fThreshold = (float)FTHRESHOLD;

        // Width of the image
        int x_last = iw - 1;
        // Height of the image
        int y_last = ih - 1;
        // X coordinate of the God Rays light source
        int x = LightScreenPosX;
        // Y coordinate of the God Rays light source
        int y = LightScreenPosY;
        // X coordinate of the God Rays light source
        int x_s = LightScreenPosX;
        // Y coordinate of the God Rays light source
        int y_s = LightScreenPosY;
        /* ************************************************************************************** *\

        Calculate the destination point of the rays and
        check to see if the rays intersect the image.

        \* ************************************************************************************** */

        // First, initialize the destinations to zero representing the lower-left
        // corner of the image, the (0,0) coordinate
        int x_dst = 0;
        int y_dst = 0;
        int x_dst_s = 0;
        int y_dst_s = 0;

        // Use the ray number as a destination pixel of the bottom border and check the range.
        int Ray = i;
        if( Ray < x_last )
        {
            // In this case, check to see if the ray is located outside of the image or
            // the ray from the left bottom corner pixel (also directed from the left).
            if( y < 0 &&
                ( x < 0 || Ray != 0 ))
            {
                // These rays do not paint any pixels, so you have no work to do
                // and you can exit the function.
                break;
            }

            // If you still have work to do, set up the destination points for the original
            // and shadow rays as a ray number.
            x_dst = Ray;
            y_dst = 0;
            x_dst_s = Ray - 1;
            y_dst_s = 0;

            // If the original ray is a corner ray, fix the shadow ray
            // destination point, because it places at another border.
            if( Ray == 0 )
            {
                x_dst_s = 0;
                y_dst_s = 1;
            }
        }

        // If the ray number is greater than the bottom border range, check other borders.
        else
        {
            // Fix the ray number by the bottom border length to exclude those
            // pixels from computation.
            Ray -= x_last;

            // Set the ray number as a destination pixel of the right border and check the range.
            if( Ray < y_last )
            {
                // To see what you need to do, check if the ray is located out from
                // the image and the ray from the right bottom corner pixel is also
                // directed from the right.
                if( x > x_last &&
                    ( y < 0 || Ray != 0 ))
                {
                    break;
                }

                // Set up destination points for original and shadow rays as the
                // ray number if there is some work to do.
                x_dst = x_last;
                y_dst = Ray;
                x_dst_s = x_last;
                y_dst_s = Ray - 1;

                // Fix the shadow ray destination point, in case the original ray is a corner ray.
                if( Ray == 0 )
                {
                    x_dst_s = x_last - 1;
                    y_dst_s = 0;
                }
            }

            // Repeat the steps for the top border if the ray number doesn't
            // correspond to the right border.
            else
            {
                Ray -= y_last;
                if( Ray < x_last )
                {
                    if( y > y_last &&
                        ( x > x_last || Ray != 0 ))
                    {
                        break;
                    }
                    x_dst = x_last - Ray;
                    y_dst = y_last;
                    x_dst_s = x_last - Ray + 1;
                    y_dst_s = y_last;
                    if( Ray == 0 )
                    {
                        x_dst_s = x_last;
                        y_dst_s = y_last - 1;
                    }
                }

                // Check left borders if the ray number doesn't correspond to the top one.
                else
                {
                    Ray -= x_last;
                    if( Ray < y_last )
                    {
                        if( x < 0 &&
                            ( y > y_last || Ray != 0 ))
                        {
                            break;
                        }
                        x_dst = 0;
                        y_dst = y_last - Ray;
                        x_dst_s = 0;
                        y_dst_s = y_last - Ray + 1;
                        if( Ray == 0 )
                        {
                            x_dst_s = 1;
                            y_dst_s = y_last;
                        }
                    }
                    else
                    {
                        // If the ray number doesn't correspond to any border, you have no
                        // more work to do, so break from the function.
                        break;
                    }
                }
            }
        }

        // Calculate start and destination points sizes for each dimension
        // and select directions as the steps size.
        int dx = x_dst - x;
        int dy = y_dst - y;
        int dx_s = x_dst_s - x;
        int dy_s = y_dst_s - y;
        int xstep = dx > 0 ? 1: 0;
        int ystep = dy > 0 ? 1: 0;
        int xstep_s = dx_s > 0 ? 1: 0;
        int ystep_s = dy_s > 0 ? 1: 0;
        if( dx < 0 )
        {
            dx = -dx;
            xstep = -1;
        }
        if( dy < 0 )
        {
            dy = -dy;
            ystep = -1;
        }
        if( dx_s < 0 )
        {
            dx_s = -dx_s;
            xstep_s = -1;
        }
        if( dy_s < 0 )
        {
            dy_s = -dy_s;
            ystep_s = -1;
        }

        // Calculate Bresenham parameters for the original ray and correct the
        // Decay value by the step length.
        float FixedDecay = 0.f;
        int di = 0;
        int di_s = 0;
        int steps = 0;

        // Select the biggest dimension of the original ray.

        // If dx is greater than or equal to dy, use dx as the steps count.
        if( dx >= dy )
        {
            // Fix Decay by the step length computed as a ray length divided by steps count.
            FixedDecay = exp( _decay * sqrt( float( dx ) *
                float( dx ) + float( dy ) * float( dy )) / float( dx ));

            // Set up the Bresenham delta error.
            di = 2 * dy - dx;

            // Crop the ray by the image borders to reduce the steps cont.

            // If the horizontal ray (dx >= dy) crosses over a left or right
            // border, you should crop it by the step count. In other words, you
            // know the first step in image borders and need to find the
            // corresponding y coordinate and the delta error value.
            if( x<0 || x>x_last )
            {
                // Select the count of steps that need to be omitted.
                int dx_crop = x<0 ? -x : x-x_last;

                // Fix the delta error value by the omitted steps on X dimension.
                int new_di = di + ( dx_crop - 1 )*2*dy;

                // Calculate the appropriate steps count for Y dimension.
                int dy_crop = new_di / ( 2*dx );

                // Check the final delta error value.
                new_di -= dy_crop*2*dx;

                // If a new delta error value is greater than 0, move by the Y
                // dimension when you cross the image range.
                if( new_di>0 )
                {
                    // In that case, increase the steps count on Y dimension by 1.
                    dy_crop++;
                }

                // Recalculate the current position and the delta error value.
                x += xstep*dx_crop;
                y += ystep*dy_crop;
                di += dx_crop*2*dy - dy_crop*2*dx;
            }

            // If the horizontal ray (dx >= dy) crosses either the bottom or top
            // border, you should crop it by the y coordinate. You know the y coordinate
            // and need to find the first corresponding step in the image borders and
            // the delta error value. */
            if( y<0 || y>y_last )
            {
                // Select the count of steps that you should omit on the Y coordinate.
                int dy_crop = y<0 ? -y : y-y_last;

                // Fix the delta error value by omitting steps on the Y dimension.
                int new_di = di - ( dy_crop - 1 )*2*dx;

                // Calculate the appropriate steps count for the X dimension.
                int dx_crop = 1 - new_di / ( 2*dy );

                // If a new delta error value is less than 0 and this value can
                // be divided by 2dy without a remainder, move by the Y dimension
                // when you cross the image range.
                if( new_di % ( 2*dy ) != 0 && new_di < 0 )
                {
                    // Increase steps count on the X dimension by 1.
                    dx_crop++;
                }

                // Since you know the right count of omitted steps in all
                // dimensions, you can recalculate the current position and the
                // delta error value.
                x += xstep*dx_crop;
                y += ystep*dy_crop;
                di += dx_crop*2*dy - dy_crop*2*dx;
            }

            // At the end of cropping the horizontal ray, calculate the correct steps count.
            steps = abs( x_dst - x );
        }

        // If dy is greater than dx, use dy as the count of steps and perform
        // exactly the same steps but with different coordinates.
        else
        {

            // Fix Decay by the step length, which is computed as the ray length divided by the steps count.
            FixedDecay = exp( _decay * sqrt( float( dx ) *
                float( dx ) + float( dy ) * float( dy )) / float( dy ));

            // Set up the Bresenham delta error.
            di = 2 * dx - dy;

            // Crop the ray by the image borders to reduce the steps cont.

            // If the vertical ray (dy > dx) crosses either a bottom or top
            // border, crop it by the step count.
            if( y<0 || y>y_last )
            {
                // Select the count of steps that should be omitted.
                int dy_crop = y<0 ? -y : y-y_last;

                // Use the omitted steps to fix the delta error value on the y dimension.
                int new_di = di + ( dy_crop - 1 )*2*dx;

                // Calculate the appropriate steps count for the X dimension.
                int dx_crop = new_di / ( 2*dy );

                // Check the final delta error value.
                new_di -= dx_crop*2*dy;

                // If a new delta error value is greater than zero,
                if( new_di>0 )
                {
                    // increase the steps count on the X dimension by 1.
                    dx_crop++;
                }

                // Recalculate the current position and the delta error value.
                x += xstep*dx_crop;
                y += ystep*dy_crop;
                di += dy_crop*2*dx - dx_crop*2*dy;
            }

            // If the vertical ray (dy > dx) crosses either a left or right
            // border, crop it on the X coordinate.
            if( x<0 || x>x_last )
            {
                // Select the count of steps that should be omitted on the X  coordinate.
                int dx_crop = x<0 ? -x : x-x_last;

                // Use omitted steps to fix the delta error value on the X dimension.
                int new_di = di - ( dx_crop - 1 )*2*dy;

                // Calculate the appropriate steps count for the Y dimension.
                int dy_crop = 1 - new_di / ( 2*dx );

                // If a new delta error value is less than 0 and this value can
                // be divided by 2dy without a remainder, then ...
                if( new_di % ( 2*dx ) != 0 && new_di < 0 )
                {
                    // increase the steps count on X dimension by 1.
                    dy_crop++;
                }

                // Recalculate the current position and the delta error value.
                x += xstep*dx_crop;
                y += ystep*dy_crop;
                di += dy_crop*2*dx - dx_crop*2*dy;
            }

            // After cropping the vertical ray, calculate the correct steps count.
            steps = abs( y_dst - y );
        }

        // Omit some steps either at the beginning or the end of the shadow ray to
        // make the steps by pixels correspond to the original ray.
        int steps_begin = 0;
        int steps_lsat = 0;

        // Crop the shadow ray by exactly the same computation as the original one.
        if( dx_s >= dy_s )
        {
            di_s = 2 * dy_s - dx_s;
            if( x_s<0 || x_s>x_last )
            {
                int dx_crop_s = x_s<0 ? -x_s : x_s-x_last;
                int new_di_s = di_s + ( dx_crop_s - 1 )*2*dy_s;
                int dy_crop_s = new_di_s / ( 2*dx_s );
                new_di_s -= dy_crop_s*2*dx_s;
                if( new_di_s>0 )
                {
                    dy_crop_s++;
                }
                x_s += xstep_s*dx_crop_s;
                y_s += ystep_s*dy_crop_s;
                di_s += dx_crop_s*2*dy_s - dy_crop_s*2*dx_s;
            }
            if( y_s<0 || y_s>y_last )
            {
                int dy_crop_s = y_s<0 ? -y_s : y_s-y_last;
                int new_di_s = di_s - ( dy_crop_s - 1 )*2*dx_s;
                int dx_crop_s = 1 - new_di_s / ( 2*dy_s );
                if( new_di_s % ( 2*dy_s ) != 0 && new_di_s < 0 )
                {
                    dx_crop_s++;
                }
                x_s += xstep_s*dx_crop_s;
                y_s += ystep_s*dy_crop_s;
                di_s += dx_crop_s*2*dy_s - dy_crop_s*2*dx_s;
            }

            // For the horizontal shadow ray, calculate the omitted steps as the
            // difference between the current position, (it can be different
            // after cropping), and the original position.
            steps_begin = xstep_s * ( x - x_s );

            // If steps at the shadow ray should be omitted (the count of
            // omitted steps is greater than 0), omit them.
            if( steps_begin > 0 )
            {
                // Calculate a new delta error value and move the start X coordinate
                // to the same position that the original ray has.
                di_s += 2 * steps_begin * dy_s;
                x_s = x;

                // To move along the Y axes, do the following steps:
                if( di_s > 2 * dy_s )
                {
                    // Correct the delta error value and ...
                    di_s -= 2 * dx_s;

                    // Move the current position by the Y dimension.
                    y_s += ystep_s;

                    // Move along the Y axes no more than one step. The distance
                    // between rays must be no greater than 1; otherwise, it is
                    // impossible to define the rays.
                }
            }

            // Define the count of steps that should be omitted at the end of
            // the shadow ray as the count of steps that should be compared,
            // (the count of shadow ray steps without steps omitted at the
            // beginning).
            steps_lsat = abs( x_dst_s - x_s ) - steps_begin;
        }
        else // Vertical shadow rays
        {
            di_s = 2 * dx_s - dy_s;
            if( y_s<0 || y_s>y_last )
            {
                int dy_crop_s = y_s<0 ? -y_s : y_s-y_last;
                int new_di_s = di_s + ( dy_crop_s - 1 )*2*dx_s;
                int dx_crop_s = new_di_s / ( 2*dy_s );
                new_di_s -= dx_crop_s*2*dy_s;
                if( new_di_s>0 )
                {
                    dx_crop_s++;
                }
                x_s += xstep_s*dx_crop_s;
                y_s += ystep_s*dy_crop_s;
                di_s += dy_crop_s*2*dx_s - dx_crop_s*2*dy_s;
            }
            if( x_s<0 || x_s>x_last )
            {
                int dx_crop_s = x_s<0 ? -x_s : x_s-x_last;
                int new_di_s = di_s - ( dx_crop_s - 1 )*2*dy_s;
                int dy_crop_s = 1 - new_di_s / ( 2*dx_s );
                if( new_di_s % ( 2*dx_s ) != 0 && new_di_s < 0 )
                {
                    dy_crop_s++;
                }
                x_s += xstep_s*dx_crop_s;
                y_s += ystep_s*dy_crop_s;
                di_s += dy_crop_s*2*dx_s - dx_crop_s*2*dy_s;
            }

            // For the vertical shadow ray, calculate the omitted steps by the
            // same computations but with different dimensions:

            // Setup the omitted steps at the beginning as the difference
            // between the current position and the original position.
            steps_begin = ystep_s * ( y - y_s );

            // Omit the first steps at the shadow ray if required by checking
            // to see if the variable steps_begin is greater than zero.
            if( steps_begin > 0 )
            {
                // Calculate a new delta error value
                di_s += 2 * steps_begin * dx_s;

                // Move the start X coordinate,
                y_s = y;
                if( di_s > 2 * dx_s )
                {
                    // correct the delta error value, and move the current
                    // position by the Y dimension.
                    di_s -= 2 * dy_s;
                    x_s += ystep_s;
                }
            }

            // Fix the count of omitted steps at the end.
            steps_lsat = abs( y_dst_s - y_s ) - steps_begin;
        }

        // Apply the Bresenham loop and perform general computation.

        // Load method parameters.
        __m128 Weight_128 = _mm_load_ps1( &_Weight );
        __m128 Decay_128 = _mm_load_ps1( &FixedDecay );
        FixedDecay = 1.f - FixedDecay;
        __m128 nDecay_128 = _mm_load_ps1( &FixedDecay );
        __m128 NExposure_128 = _mm_set_ps1( _InvExposure );
        __m128 summ_128 = _mm_setzero_ps( );
        __m128 One_128 = _mm_set_ps1( _fThreshold );

        // Load the first pixel of the ray.
        __m128 sample_128 = _mm_load_ps(
            (float*)
            (
            (char*)inBuf +
            x * 4 * sizeof( float ) +
            y * iw * 4 * sizeof(float))
            );

        // Apply the exposure to it to scale color values to appropriate ranges.
        sample_128 = _mm_mul_ps( sample_128, NExposure_128 );

        // Check to see if this pixel can be used in a sum.
        ///if( _mm_movemask_ps( _mm_cmpgt_ps( sample_128, One_128 )) & 7 )
        {
            // Update the sum if required.
            summ_128 = _mm_mul_ps( sample_128, nDecay_128 );
        }

        // Check to see if the result can be written to the output pixel (the pixel is not
        // shaded by the shadow ray and it is not the first ray that should
        // paint the sun position).
        if( x!=x_s || y!=y_s || ( x_dst==0 && y_dst==0 &&
            x==LightScreenPosX && y==LightScreenPosY ))
        {
            // Add the current sum value corrected by the Weight parameter to
            // the output buffer if possible.
            __m128 answer_128 = _mm_mul_ps( summ_128, Weight_128 );
            //sample_128 = _mm_load_ps(outBuf+y*iw*sizeof(float)*4+x*sizeof(float)*4);
            answer_128 = _mm_add_ps( answer_128, sample_128 );
            _mm_store_ps(
                (float*)(
                (char*)outBuf +
                x * 4 * sizeof( float ) +
                y*iw*sizeof(float)*4 ),
                answer_128
                );
        }

        // In the main loop, go along the original ray.
        for( int i = 0; i < steps; i++ )
        {
            // Make steps in the Bresenham loop for the original ray.
            if( dx >= dy )
            {
                x += xstep;
                if( di >= 0 )
                {
                    y += ystep;
                    di -= 2*dx;
                }
                di += 2 * dy;
            }
            else
            {
                y += ystep;
                if( di >= 0 )
                {
                    x += xstep;
                    di -= 2*dy;
                }
                di += 2 * dx;
            }

            // Make steps for the shadow rays if if they should not be omitted.
            if( steps_begin >= 0 )
            {
                if( dx_s >= dy_s )
                {
                    x_s += xstep_s;
                    if( di_s >= 0 )
                    {
                        y_s += ystep_s;
                        di_s -= 2*dx_s;
                    }
                    di_s += 2 * dy_s;
                }
                else
                {
                    y_s += ystep_s;
                    if( di_s >= 0 )
                    {
                        x_s += xstep_s;
                        di_s -= 2*dy_s;
                    }
                    di_s += 2*dx_s;
                }
            }
            else
            {
                steps_begin++;
            }

            // For each step, load the next pixel of the ray.
            sample_128 = _mm_load_ps(
                (float*)
                (
                (char*)inBuf+
                y*iw*sizeof(float)*4+
                x*sizeof(float)*4 )
                );

            // Apply the exposure to it to scale color values to appropriate ranges.
            summ_128   = _mm_mul_ps( summ_128, Decay_128 );

            // Update the sum
            sample_128 = _mm_mul_ps( sample_128, NExposure_128);

            // check if this pixel can be used in a sum and
            ///if(( _mm_movemask_ps( _mm_cmpgt_ps( sample_128, One_128 )) & 7 ) != 0)
            {
                // update the sum if required.
                sample_128 = _mm_mul_ps( sample_128, nDecay_128 );
                summ_128   = _mm_add_ps( summ_128, sample_128 );
            }

            // Check if it is possible to write the result to the output pixel.
            if( x != x_s || y != y_s || i >= steps_lsat )
            {
                // Update the output buffer by the current sum value
                // corrected by the Weight parameter.
                __m128 answer_128 = _mm_mul_ps( summ_128, Weight_128 );

                sample_128 =
                    _mm_load_ps(
                    (float*)
                    (
                    (char*)inBuf +
                    x * 4 * sizeof( float ) +
                    y*iw*sizeof(float)*4
                    )
                    );

                if(blend == 1){
                    answer_128 = _mm_add_ps( answer_128, sample_128 );
                    _mm_store_ps(
                        (float*)(
                        (char*)outBuf +
                        x * 4 * sizeof( float ) +
                        y*iw*sizeof(float)*4 ),
                        answer_128
                        );
                }
                else
                {
                    _mm_store_ps(
                        (float*)(
                        (char*)outBuf +
                        x * 4 * sizeof( float ) +
                        y*iw*sizeof(float)*4 ),
                        answer_128
                        );
                }
            }
        }
    }
}
