/*
 * Copyright (C) 2017 Dimitri Bouron <bouron.d@gmail.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVFILTER_DRAWBOX_OPENCL_KERNEL_H
#define AVFILTER_DRAWBOX_OPENCL_KERNEL_H

#include "libavutil/opencl.h"


const char *ff_kernel_drawbox_opencl = AV_OPENCL_KERNEL(

enum { Y, U, V, A };

kernel void drawbox(global unsigned char *src,
                    global unsigned char *dst,
                    constant unsigned char *yuv,
                    int have_alpha,
                    int invert_color,
                    int thickness,
                    int width,
                    int height,
                    int cw,
                    int ch,
                    int hsub,
                    int vsub,
                    int bx,
                    int by,
                    int bw,
                    int bh)
{
    global unsigned char *dst_channels[4];
    global unsigned char *src_channels[4];

    int x = get_global_id(0);
    int y = get_global_id(1);
    int u_offset = width * height;
    int v_offset = u_offset + cw * ch;
    int a_offset = v_offset + cw * ch;
    int ya_pos = x + y * width;
    int uv_pos = (x >> hsub) + (y >> vsub) * cw;
    bool cond = y > by && y < by + bh && x > bx && x < bx + bw &&
                ((y - by < thickness) || (by + bh - 1 - y < thickness) ||
                 (x - bx < thickness) || (bx + bw - 1 - x < thickness));

    dst_channels[0] = dst + ya_pos;
    dst_channels[1] = dst + uv_pos + u_offset;
    dst_channels[2] = dst + uv_pos + v_offset;

    src_channels[0] = src + ya_pos;
    src_channels[1] = src + uv_pos + u_offset;
    src_channels[2] = src + uv_pos + v_offset;

    *dst_channels[0] = *src_channels[0];
    *dst_channels[1] = *src_channels[1];
    *dst_channels[2] = *src_channels[2];
    if (have_alpha) {
        dst_channels[3] = dst + ya_pos + a_offset;
        src_channels[3] = src + ya_pos + a_offset;

        *dst_channels[3] = *src_channels[3];
        if (cond) {
            if (invert_color) {
                *dst_channels[0] = 0xff - *src_channels[0];
            }
            else {
                *dst_channels[0] = yuv[Y];
                *dst_channels[1] = yuv[U];
                *dst_channels[2] = yuv[V];
                *dst_channels[3] = yuv[A];
            }
        }
    }
    else {
        if (cond) {
            if (invert_color) {
                *dst_channels[0] = 0xff - *src_channels[0];
            }
            else {
                float alpha = convert_float(yuv[A]) / 255.0;
                float src_y = convert_float(*src_channels[0]);
                float src_u = convert_float(*src_channels[1]);
                float src_v = convert_float(*src_channels[2]);

                *dst_channels[0] = convert_uchar((1.0 - alpha) * src_y + alpha * convert_float(yuv[Y]));
                *dst_channels[1] = convert_uchar((1.0 - alpha) * src_u + alpha * convert_float(yuv[U]));
                *dst_channels[2] = convert_uchar((1.0 - alpha) * src_v + alpha * convert_float(yuv[V]));
            }
        }
    }
}

inline int pixel_belongs_to_grid(int x,
                                 int y,
                                 int thickness,
                                 int gx,
                                 int gy,
                                 int gw,
                                 int gh)
{
    // x is horizontal (width) coord,
    // y is vertical (height) coord
    int x_modulo;
    int y_modulo;

    // Abstract from the offset
    x -= gx;
    y -= gy;

    x_modulo = x % gw;
    y_modulo = y % gh;

    // If x or y got negative, fix values to preserve logics
    if (x_modulo < 0)
        x_modulo += gw;
    if (y_modulo < 0)
        y_modulo += gh;

    return x_modulo < thickness  // Belongs to vertical line
        || y_modulo < thickness;  // Belongs to horizontal line
}

kernel void drawgrid(global unsigned char *src,
                     global unsigned char *dst,
                     constant unsigned char *yuv,
                     int have_alpha,
                     int invert_color,
                     int thickness,
                     int width,
                     int height,
                     int cw,
                     int ch,
                     int hsub,
                     int vsub,
                     int gx,
                     int gy,
                     int gw,
                     int gh)
{
    global unsigned char *dst_channels[4];
    global unsigned char *src_channels[4];

    int x = get_global_id(0);
    int y = get_global_id(1);
    int u_offset = width * height;
    int v_offset = u_offset + cw * ch;
    int a_offset = v_offset + cw * ch;
    int ya_pos = x + y * width;
    int uv_pos = (x >> hsub) + (y >> vsub) * cw;
    bool cond = pixel_belongs_to_grid(x, y, thickness, gx, gy, gw, gh);

    dst_channels[0] = dst + ya_pos;
    dst_channels[1] = dst + uv_pos + u_offset;
    dst_channels[2] = dst + uv_pos + v_offset;

    src_channels[0] = src + ya_pos;
    src_channels[1] = src + uv_pos + u_offset;
    src_channels[2] = src + uv_pos + v_offset;

    *dst_channels[0] = *src_channels[0];
    *dst_channels[1] = *src_channels[1];
    *dst_channels[2] = *src_channels[2];
    if (have_alpha) {
        dst_channels[3] = dst + ya_pos + a_offset;
        src_channels[3] = src + ya_pos + a_offset;

        *dst_channels[3] = *src_channels[3];
        if (cond) {
            if (invert_color) {
                *dst_channels[0] = 0xff - *src_channels[0];
            }
            else {
                *dst_channels[0] = yuv[Y];
                *dst_channels[1] = yuv[U];
                *dst_channels[2] = yuv[V];
                *dst_channels[3] = yuv[A];
            }
        }
    }
    else {
        if (cond) {
            if (invert_color) {
                *dst_channels[0] = 0xff - *src_channels[0];
            }
            else {
                float alpha = convert_float(yuv[A]) / 255.0;
                float src_y = convert_float(*src_channels[0]);
                float src_u = convert_float(*src_channels[1]);
                float src_v = convert_float(*src_channels[2]);

                *dst_channels[0] = convert_uchar((1.0 - alpha) * src_y + alpha * convert_float(yuv[Y]));
                *dst_channels[1] = convert_uchar((1.0 - alpha) * src_u + alpha * convert_float(yuv[U]));
                *dst_channels[2] = convert_uchar((1.0 - alpha) * src_v + alpha * convert_float(yuv[V]));
            }
        }
    }
}
);

#endif /* AVFILTER_DRAWBOX_OPENCL_KERNEL_H */
