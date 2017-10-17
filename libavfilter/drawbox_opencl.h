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

#ifndef AVFILTER_DRAWBOX_OPENCL_H
#define AVFILTER_DRAWBOX_OPENCL_H

#include "drawbox.h"

int ff_opencl_drawbox_init(AVFilterContext *ctx);

void ff_opencl_drawbox_uninit(AVFilterContext *ctx);

int ff_opencl_drawbox_process_inout_buf(AVFilterContext *ctx, AVFrame *frame, AVFrame *out);

int ff_opencl_apply_drawbox(AVFilterContext *ctx, AVFrame *frame, AVFrame *out);

int ff_opencl_apply_drawgrid(AVFilterContext *ctx, AVFrame *frame, AVFrame *out);

#endif /* AVFILTER_DRAWBOX_OPENCL_H */
