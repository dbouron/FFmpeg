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

#ifndef AVFILTER_DRAWBOX_H
#define AVFILTER_DRAWBOX_H

#include "avfilter.h"
#if CONFIG_OPENCL
#include "libavutil/opencl.h"
#endif

#if CONFIG_OPENCL

typedef struct DrawBoxOpenclContext {
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel_drawbox;
    cl_kernel kernel_drawgrid;
    int plane_size[8];
    int plane_num;
    cl_mem cl_buf;
    size_t cl_buf_size;
} DrawBoxOpenclContext;

#endif

typedef struct DrawBoxContext {
    const AVClass *class;
    int x, y, w, h;
    int thickness;
    char *color_str;
    unsigned char yuv_color[4];
    int invert_color; ///< invert luma color
    int vsub, hsub;   ///< chroma subsampling
    char *x_expr, *y_expr; ///< expression for x and y
    char *w_expr, *h_expr; ///< expression for width and height
    char *t_expr;          ///< expression for thickness
    int have_alpha;
    int opencl;
#if CONFIG_OPENCL
    DrawBoxOpenclContext opencl_ctx;
#endif
    int (* apply_drawbox)(AVFilterContext *ctx, AVFrame *in);
} DrawBoxContext;

#endif /* AVFILTER_DRAWBOX_H */
