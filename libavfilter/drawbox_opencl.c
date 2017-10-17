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

/**
 * @file
 * drawbox input video
 */

#include "drawbox_opencl.h"
#include "libavutil/opencl_internal.h"

#define PLANE_NUM 3
#define ROUND_TO_16(a) (((((a) - 1)/16)+1)*16)

int ff_opencl_apply_drawbox(AVFilterContext *ctx, AVFrame *frame, AVFrame *out)
{
    int ret;
    AVFilterLink *link = ctx->inputs[0];
    DrawBoxContext *drawbox = ctx->priv;
    cl_int status;
    FFOpenclParam kernel = {0};
    int width = link->w;
    int height = link->h;
    int cw = AV_CEIL_RSHIFT(link->w, drawbox->hsub);
    int ch = AV_CEIL_RSHIFT(link->h, drawbox->vsub);
    size_t globalWorkSize2d[2];
    size_t localWorkSize2d[2] = {16, 16};

    globalWorkSize2d[0] = (size_t)ROUND_TO_16(width);
    globalWorkSize2d[1] = (size_t)ROUND_TO_16(height);

    kernel.ctx = ctx;
    kernel.kernel = drawbox->opencl_ctx.kernel_drawbox;
    ret = avpriv_opencl_set_parameter(&kernel,
                                      FF_OPENCL_PARAM_INFO(drawbox->opencl_ctx.cl_inbuf),
                                      FF_OPENCL_PARAM_INFO(drawbox->opencl_ctx.cl_outbuf),
                                      FF_OPENCL_PARAM_INFO(drawbox->opencl_ctx.cl_yuvbuf),
                                      FF_OPENCL_PARAM_INFO(drawbox->have_alpha),
                                      FF_OPENCL_PARAM_INFO(drawbox->invert_color),
                                      FF_OPENCL_PARAM_INFO(drawbox->thickness),
                                      FF_OPENCL_PARAM_INFO(width),
                                      FF_OPENCL_PARAM_INFO(height),
                                      FF_OPENCL_PARAM_INFO(cw),
                                      FF_OPENCL_PARAM_INFO(ch),
                                      FF_OPENCL_PARAM_INFO(drawbox->hsub),
                                      FF_OPENCL_PARAM_INFO(drawbox->vsub),
                                      FF_OPENCL_PARAM_INFO(drawbox->x),
                                      FF_OPENCL_PARAM_INFO(drawbox->y),
                                      FF_OPENCL_PARAM_INFO(drawbox->w),
                                      FF_OPENCL_PARAM_INFO(drawbox->h),
                                      NULL);
    if (ret < 0)
        return ret;

    status = clEnqueueNDRangeKernel(drawbox->opencl_ctx.command_queue,
                                    kernel.kernel, 2, NULL, globalWorkSize2d, localWorkSize2d,
                                    0, NULL, NULL);
    if (status != CL_SUCCESS) {
        av_log(ctx, AV_LOG_ERROR, "OpenCL run kernel error occurred: %s\n", av_opencl_errstr(status));
        return AVERROR_EXTERNAL;
    }
    //blocking map is suffficient, no need for clFinish
    //clFinish(drawbox->opencl_ctx.command_queue);

    return av_opencl_buffer_read_image(out->data, drawbox->opencl_ctx.out_plane_size,
                                       drawbox->opencl_ctx.plane_num, drawbox->opencl_ctx.cl_outbuf,
                                       drawbox->opencl_ctx.cl_outbuf_size);
                                       return 0;
}

int ff_opencl_apply_drawgrid(AVFilterContext *ctx, AVFrame *frame, AVFrame *out)
{
    int ret;
    AVFilterLink *link = ctx->inputs[0];
    DrawBoxContext *drawgrid = ctx->priv;
    cl_int status;
    FFOpenclParam kernel = {0};
    int width = link->w;
    int height = link->h;
    int cw = AV_CEIL_RSHIFT(link->w, drawgrid->hsub);
    int ch = AV_CEIL_RSHIFT(link->h, drawgrid->vsub);
    size_t globalWorkSize2d[2];
    size_t localWorkSize2d[2] = {16, 16};

    globalWorkSize2d[0] = (size_t)ROUND_TO_16(width);
    globalWorkSize2d[1] = (size_t)ROUND_TO_16(height);

    kernel.ctx = ctx;
    kernel.kernel = drawgrid->opencl_ctx.kernel_drawgrid;
    ret = avpriv_opencl_set_parameter(&kernel,
                                      FF_OPENCL_PARAM_INFO(drawgrid->opencl_ctx.cl_inbuf),
                                      FF_OPENCL_PARAM_INFO(drawgrid->opencl_ctx.cl_outbuf),
                                      FF_OPENCL_PARAM_INFO(drawgrid->opencl_ctx.cl_yuvbuf),
                                      FF_OPENCL_PARAM_INFO(drawgrid->have_alpha),
                                      FF_OPENCL_PARAM_INFO(drawgrid->invert_color),
                                      FF_OPENCL_PARAM_INFO(drawgrid->thickness),
                                      FF_OPENCL_PARAM_INFO(width),
                                      FF_OPENCL_PARAM_INFO(height),
                                      FF_OPENCL_PARAM_INFO(cw),
                                      FF_OPENCL_PARAM_INFO(ch),
                                      FF_OPENCL_PARAM_INFO(drawgrid->hsub),
                                      FF_OPENCL_PARAM_INFO(drawgrid->vsub),
                                      FF_OPENCL_PARAM_INFO(drawgrid->x),
                                      FF_OPENCL_PARAM_INFO(drawgrid->y),
                                      FF_OPENCL_PARAM_INFO(drawgrid->w),
                                      FF_OPENCL_PARAM_INFO(drawgrid->h),
                                      NULL);
    if (ret < 0)
        return ret;

    status = clEnqueueNDRangeKernel(drawgrid->opencl_ctx.command_queue, kernel.kernel, 2, NULL,
                                    globalWorkSize2d, localWorkSize2d, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        av_log(ctx, AV_LOG_ERROR, "OpenCL run kernel error occurred: %s\n", av_opencl_errstr(status));
        return AVERROR_EXTERNAL;
    }
    //blocking map is suffficient, no need for clFinish
    //clFinish(drawgrid->opencl_ctx.command_queue);

    return av_opencl_buffer_read_image(out->data, drawgrid->opencl_ctx.out_plane_size,
                                       drawgrid->opencl_ctx.plane_num, drawgrid->opencl_ctx.cl_outbuf,
                                       drawgrid->opencl_ctx.cl_outbuf_size);
                                       return 0;
}

int ff_opencl_drawbox_init(AVFilterContext *ctx)
{
    int ret = 0;
    DrawBoxContext *drawbox = ctx->priv;

    ret = av_opencl_init(NULL);
    if (ret < 0)
        return ret;

    drawbox->opencl_ctx.plane_num = PLANE_NUM;
    drawbox->opencl_ctx.command_queue = av_opencl_get_command_queue();
    if (!drawbox->opencl_ctx.command_queue) {
        av_log(ctx, AV_LOG_ERROR, "Unable to get OpenCL command queue in filter 'drawbox/drawgrid'\n");
        return AVERROR(EINVAL);
    }
    drawbox->opencl_ctx.program = av_opencl_compile("drawbox", NULL);
    if (!drawbox->opencl_ctx.program) {
        av_log(ctx, AV_LOG_ERROR, "OpenCL failed to compile program 'drawbox/drawgrid'\n");
        return AVERROR(EINVAL);
    }

    if (!drawbox->opencl_ctx.kernel_drawbox) {
        drawbox->opencl_ctx.kernel_drawbox = clCreateKernel(drawbox->opencl_ctx.program, "drawbox", &ret);
        if (ret < 0) {
            av_log(ctx, AV_LOG_ERROR, "OpenCL failed to create kernel 'drawbox'\n");
            return ret;
        }
    }
    if (!drawbox->opencl_ctx.kernel_drawgrid) {
        drawbox->opencl_ctx.kernel_drawgrid = clCreateKernel(drawbox->opencl_ctx.program, "drawgrid", &ret);
        if (ret < 0) {
            av_log(ctx, AV_LOG_ERROR, "OpenCL failed to create kernel 'drawgrid'\n");
            return ret;
        }
    }
    return ret;
}

void ff_opencl_drawbox_uninit(AVFilterContext *ctx)
{
    DrawBoxContext *drawbox = ctx->priv;

    av_opencl_buffer_release(&drawbox->opencl_ctx.cl_inbuf);
    av_opencl_buffer_release(&drawbox->opencl_ctx.cl_outbuf);
    av_opencl_buffer_release(&drawbox->opencl_ctx.cl_yuvbuf);
    clReleaseKernel(drawbox->opencl_ctx.kernel_drawbox);
    clReleaseKernel(drawbox->opencl_ctx.kernel_drawgrid);
    clReleaseProgram(drawbox->opencl_ctx.program);
    drawbox->opencl_ctx.command_queue = NULL;
    av_opencl_uninit();
}

int ff_opencl_drawbox_process_inout_buf(AVFilterContext *ctx, AVFrame *frame, AVFrame *out)
{
    int ret = 0;
    AVFilterLink *link = ctx->inputs[0];
    DrawBoxContext *drawbox = ctx->priv;
    int ch = AV_CEIL_RSHIFT(link->h, drawbox->vsub);

    if (!drawbox->opencl_ctx.cl_inbuf || !drawbox->opencl_ctx.cl_outbuf
        || !drawbox->opencl_ctx.cl_yuvbuf) {
        drawbox->opencl_ctx.in_plane_size[0] = (frame->linesize[0] * frame->height);
        drawbox->opencl_ctx.in_plane_size[1] = (frame->linesize[1] * ch);
        drawbox->opencl_ctx.in_plane_size[2] = (frame->linesize[2] * ch);
        drawbox->opencl_ctx.cl_inbuf_size = drawbox->opencl_ctx.in_plane_size[0] +
                                            drawbox->opencl_ctx.in_plane_size[1] +
                                            drawbox->opencl_ctx.in_plane_size[2];

        drawbox->opencl_ctx.out_plane_size[0] = (frame->linesize[0] * frame->height);
        drawbox->opencl_ctx.out_plane_size[1] = (frame->linesize[1] * ch);
        drawbox->opencl_ctx.out_plane_size[2] = (frame->linesize[2] * ch);
        drawbox->opencl_ctx.cl_outbuf_size = drawbox->opencl_ctx.out_plane_size[0] +
                                             drawbox->opencl_ctx.out_plane_size[1] +
                                             drawbox->opencl_ctx.out_plane_size[2];
        drawbox->opencl_ctx.cl_yuvbuf_size = 4;

        if (!drawbox->opencl_ctx.cl_inbuf) {
            ret = av_opencl_buffer_create(&drawbox->opencl_ctx.cl_inbuf,
                                          drawbox->opencl_ctx.cl_inbuf_size,
                                          CL_MEM_READ_ONLY, NULL);
            if (ret < 0)
                return ret;
        }
        if (!drawbox->opencl_ctx.cl_outbuf) {
            ret = av_opencl_buffer_create(&drawbox->opencl_ctx.cl_outbuf,
                                          drawbox->opencl_ctx.cl_outbuf_size,
                                          CL_MEM_WRITE_ONLY, NULL);
            if (ret < 0)
                return ret;
        }
        if (!drawbox->opencl_ctx.cl_yuvbuf) {
            ret = av_opencl_buffer_create(&drawbox->opencl_ctx.cl_yuvbuf,
                                          drawbox->opencl_ctx.cl_yuvbuf_size,
                                          CL_MEM_READ_ONLY, NULL);
            if (ret < 0)
                return ret;
        }
    }
    ret = av_opencl_buffer_write(drawbox->opencl_ctx.cl_yuvbuf, drawbox->yuv_color,
                                 drawbox->opencl_ctx.cl_yuvbuf_size);
    if (ret < 0)
        return ret;
    return av_opencl_buffer_write_image(drawbox->opencl_ctx.cl_inbuf,
                                        drawbox->opencl_ctx.cl_inbuf_size,
                                        0, frame->data, drawbox->opencl_ctx.in_plane_size,
                                        drawbox->opencl_ctx.plane_num);
}
