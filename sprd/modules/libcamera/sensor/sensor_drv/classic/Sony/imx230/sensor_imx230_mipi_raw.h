/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * V2.0
 */
#ifndef _SENSOR_IMX230_MIPI_RAW_
#define _SENSOR_IMX230_MIPI_RAW_

#include "cutils/properties.h"
#include <utils/Log.h>
#include "sensor.h"
#include "jpeg_exif_header.h"
#include "sensor_drv_u.h"
#include "sensor_raw.h"

#include "parameters/sensor_imx230_raw_param_v3.c"
#include "parameters/sensor_imx230_otp_truly.h"

#define SENSOR_NAME "imx230_mipi_raw"
#define I2C_SLAVE_ADDR 0x20 /* 16bit slave address*/
#define VENDOR_NUM 1

#define BINNING_FACTOR 2
#define imx230_PID_ADDR 0x0016
#define imx230_PID_VALUE 0x02
#define imx230_VER_ADDR 0x0017
#define imx230_VER_VALUE 0x30

/* sensor parameters begin */
/* effective sensor output image size */
#define SNAPSHOT_WIDTH 4272  // 4160 //5344
#define SNAPSHOT_HEIGHT 3212 // 3120 //4016
#define PREVIEW_WIDTH 2672
#define PREVIEW_HEIGHT 2008

/*Mipi output*/
#define LANE_NUM 4
#define RAW_BITS 10

#define SNAPSHOT_MIPI_PER_LANE_BPS 1419
#define PREVIEW_MIPI_PER_LANE_BPS 800

/* please ref your spec */
#define FRAME_OFFSET 10
#define SENSOR_MAX_GAIN 0xF0
#define SENSOR_BASE_GAIN 0x20
#define SENSOR_MIN_SHUTTER 4

/* isp parameters, please don't change it*/
#if defined(CONFIG_CAMERA_ISP_VERSION_V3) ||                                   \
    defined(CONFIG_CAMERA_ISP_VERSION_V4)
#define ISP_BASE_GAIN 0x80
#else
#define ISP_BASE_GAIN 0x10
#endif

/* please don't change it */
#define EX_MCLK 24

static const SENSOR_REG_T imx230_init_setting[] = {
    {0x0136, 0x18}, {0x0137, 0x00}, {0x4800, 0x0E}, {0x4890, 0x01},
    {0x4D1E, 0x01}, {0x4D1F, 0xFF}, {0x4FA0, 0x00}, {0x4FA1, 0x00},
    {0x4FA2, 0x00}, {0x4FA3, 0x83}, {0x6153, 0x01}, {0x6156, 0x01},
    {0x69BB, 0x01}, {0x69BC, 0x05}, {0x69BD, 0x05}, {0x69C1, 0x00},
    {0x69C4, 0x01}, {0x69C6, 0x01}, {0x7300, 0x00}, {0x9009, 0x1A},
    {0xB040, 0x90}, {0xB041, 0x14}, {0xB042, 0x6B}, {0xB043, 0x43},
    {0xB044, 0x63}, {0xB045, 0x2A}, {0xB046, 0x68}, {0xB047, 0x06},
    {0xB048, 0x68}, {0xB049, 0x07}, {0xB04A, 0x68}, {0xB04B, 0x04},
    {0xB04C, 0x68}, {0xB04D, 0x05}, {0xB04E, 0x68}, {0xB04F, 0x16},
    {0xB050, 0x68}, {0xB051, 0x17}, {0xB052, 0x68}, {0xB053, 0x74},
    {0xB054, 0x68}, {0xB055, 0x75}, {0xB056, 0x68}, {0xB057, 0x76},
    {0xB058, 0x68}, {0xB059, 0x77}, {0xB05A, 0x68}, {0xB05B, 0x7A},
    {0xB05C, 0x68}, {0xB05D, 0x7B}, {0xB05E, 0x68}, {0xB05F, 0x0A},
    {0xB060, 0x68}, {0xB061, 0x0B}, {0xB062, 0x68}, {0xB063, 0x08},
    {0xB064, 0x68}, {0xB065, 0x09}, {0xB066, 0x68}, {0xB067, 0x0E},
    {0xB068, 0x68}, {0xB069, 0x0F}, {0xB06A, 0x68}, {0xB06B, 0x0C},
    {0xB06C, 0x68}, {0xB06D, 0x0D}, {0xB06E, 0x68}, {0xB06F, 0x13},
    {0xB070, 0x68}, {0xB071, 0x12}, {0xB072, 0x90}, {0xB073, 0x0E},
    {0xD000, 0xDA}, {0xD001, 0xDA}, {0xD002, 0xAF}, {0xD003, 0xE1},
    {0xD004, 0x55}, {0xD005, 0x34}, {0xD006, 0x21}, {0xD007, 0x00},
    {0xD008, 0x1C}, {0xD009, 0x80}, {0xD00A, 0xFE}, {0xD00B, 0xC5},
    {0xD00C, 0x55}, {0xD00D, 0xDC}, {0xD00E, 0xB6}, {0xD00F, 0x00},
    {0xD010, 0x31}, {0xD011, 0x02}, {0xD012, 0x4A}, {0xD013, 0x0E},
    {0xD014, 0x55}, {0xD015, 0xF0}, {0xD016, 0x1B}, {0xD017, 0x00},
    {0xD018, 0xFA}, {0xD019, 0x2C}, {0xD01A, 0xF1}, {0xD01B, 0x7E},
    {0xD01C, 0x55}, {0xD01D, 0x1C}, {0xD01E, 0xD8}, {0xD01F, 0x00},
    {0xD020, 0x76}, {0xD021, 0xC1}, {0xD022, 0xBF}, {0xD044, 0x40},
    {0xD045, 0xBA}, {0xD046, 0x70}, {0xD047, 0x47}, {0xD048, 0xC0},
    {0xD049, 0xBA}, {0xD04A, 0x70}, {0xD04B, 0x47}, {0xD04C, 0x82},
    {0xD04D, 0xF6}, {0xD04E, 0xDA}, {0xD04F, 0xFA}, {0xD050, 0x00},
    {0xD051, 0xF0}, {0xD052, 0x02}, {0xD053, 0xF8}, {0xD054, 0x81},
    {0xD055, 0xF6}, {0xD056, 0xCE}, {0xD057, 0xFD}, {0xD058, 0x10},
    {0xD059, 0xB5}, {0xD05A, 0x0D}, {0xD05B, 0x48}, {0xD05C, 0x40},
    {0xD05D, 0x7A}, {0xD05E, 0x01}, {0xD05F, 0x28}, {0xD060, 0x15},
    {0xD061, 0xD1}, {0xD062, 0x0C}, {0xD063, 0x49}, {0xD064, 0x0C},
    {0xD065, 0x46}, {0xD066, 0x40}, {0xD067, 0x3C}, {0xD068, 0x48},
    {0xD069, 0x8A}, {0xD06A, 0x62}, {0xD06B, 0x8A}, {0xD06C, 0x80},
    {0xD06D, 0x1A}, {0xD06E, 0x8A}, {0xD06F, 0x89}, {0xD070, 0x00},
    {0xD071, 0xB2}, {0xD072, 0x10}, {0xD073, 0x18}, {0xD074, 0x0A},
    {0xD075, 0x46}, {0xD076, 0x20}, {0xD077, 0x32}, {0xD078, 0x12},
    {0xD079, 0x88}, {0xD07A, 0x90}, {0xD07B, 0x42}, {0xD07C, 0x00},
    {0xD07D, 0xDA}, {0xD07E, 0x10}, {0xD07F, 0x46}, {0xD080, 0x80},
    {0xD081, 0xB2}, {0xD082, 0x88}, {0xD083, 0x81}, {0xD084, 0x84},
    {0xD085, 0xF6}, {0xD086, 0x06}, {0xD087, 0xF8}, {0xD088, 0xE0},
    {0xD089, 0x67}, {0xD08A, 0x85}, {0xD08B, 0xF6}, {0xD08C, 0x4B},
    {0xD08D, 0xFC}, {0xD08E, 0x10}, {0xD08F, 0xBD}, {0xD090, 0x00},
    {0xD091, 0x18}, {0xD092, 0x1E}, {0xD093, 0x78}, {0xD094, 0x00},
    {0xD095, 0x18}, {0xD096, 0x17}, {0xD097, 0x98}, {0x5869, 0x01},

    {0x68A9, 0x00}, {0x68C5, 0x00}, {0x68DF, 0x00}, {0x6953, 0x01},
    {0x6962, 0x3A}, {0x69CD, 0x3A}, {0x9258, 0x00}, {0x933A, 0x02},
    {0x933B, 0x02}, {0x934B, 0x1B}, {0x934C, 0x0A}, {0x9356, 0x8C},
    {0x9357, 0x50}, {0x9358, 0x1B}, {0x9359, 0x8C}, {0x935A, 0x1B},
    {0x935B, 0x0A}, {0x940D, 0x07}, {0x940E, 0x07}, {0x9414, 0x06},
    {0x945B, 0x07}, {0x945D, 0x07}, {0x9901, 0x35}, {0x9903, 0x23},
    {0x9905, 0x23}, {0x9906, 0x00}, {0x9907, 0x31}, {0x9908, 0x00},
    {0x9909, 0x1B}, {0x990A, 0x00}, {0x990B, 0x15}, {0x990D, 0x3F},
    {0x990F, 0x3F}, {0x9911, 0x3F}, {0x9913, 0x64}, {0x9915, 0x64},
    {0x9917, 0x64}, {0x9919, 0x50}, {0x991B, 0x60}, {0x991D, 0x65},
    {0x991F, 0x01}, {0x9921, 0x01}, {0x9923, 0x01}, {0x9925, 0x23},
    {0x9927, 0x23}, {0x9929, 0x23}, {0x992B, 0x2F}, {0x992D, 0x1A},
    {0x992F, 0x14}, {0x9931, 0x3F}, {0x9933, 0x3F}, {0x9935, 0x3F},
    {0x9937, 0x6B}, {0x9939, 0x7C}, {0x993B, 0x81}, {0x9943, 0x0F},
    {0x9945, 0x0F}, {0x9947, 0x0F}, {0x9949, 0x0F}, {0x994B, 0x0F},
    {0x994D, 0x0F}, {0x994F, 0x42}, {0x9951, 0x0F}, {0x9953, 0x0B},
    {0x9955, 0x5A}, {0x9957, 0x13}, {0x9959, 0x0C}, {0x995A, 0x00},
    {0x995B, 0x00}, {0x995C, 0x00}, {0x996B, 0x00}, {0x996D, 0x10},
    {0x996F, 0x10}, {0x9971, 0xC8}, {0x9973, 0x32}, {0x9975, 0x04},
    {0x9976, 0x0A}, {0x99B0, 0x20}, {0x99B1, 0x20}, {0x99B2, 0x20},
    {0x99C6, 0x6E}, {0x99C7, 0x6E}, {0x99C8, 0x6E}, {0x9A1F, 0x0A},
    {0x9AB0, 0x20}, {0x9AB1, 0x20}, {0x9AB2, 0x20}, {0x9AC6, 0x6E},
    {0x9AC7, 0x6E}, {0x9AC8, 0x6E}, {0x9B01, 0x35}, {0x9B03, 0x14},
    {0x9B05, 0x14}, {0x9B07, 0x31}, {0x9B08, 0x01}, {0x9B09, 0x1B},
    {0x9B0A, 0x01}, {0x9B0B, 0x15}, {0x9B0D, 0x1E}, {0x9B0F, 0x1E},
    {0x9B11, 0x1E}, {0x9B13, 0x64}, {0x9B15, 0x64}, {0x9B17, 0x64},
    {0x9B19, 0x50}, {0x9B1B, 0x60}, {0x9B1D, 0x65}, {0x9B1F, 0x01},
    {0x9B21, 0x01}, {0x9B23, 0x01}, {0x9B25, 0x14}, {0x9B27, 0x14},
    {0x9B29, 0x14}, {0x9B2B, 0x2F}, {0x9B2D, 0x1A}, {0x9B2F, 0x14},
    {0x9B31, 0x1E}, {0x9B33, 0x1E}, {0x9B35, 0x1E}, {0x9B37, 0x6B},
    {0x9B39, 0x7C}, {0x9B3B, 0x81}, {0x9B43, 0x0F}, {0x9B45, 0x0F},
    {0x9B47, 0x0F}, {0x9B49, 0x0F}, {0x9B4B, 0x0F}, {0x9B4D, 0x0F},
    {0x9B4F, 0x2D}, {0x9B51, 0x0B}, {0x9B53, 0x08}, {0x9B55, 0x40},
    {0x9B57, 0x0D}, {0x9B59, 0x08}, {0x9B5A, 0x00}, {0x9B5B, 0x00},
    {0x9B5C, 0x00}, {0x9B5D, 0x08}, {0x9B5E, 0x0E}, {0x9B60, 0x08},
    {0x9B61, 0x0E}, {0x9B6B, 0x00}, {0x9B6D, 0x10}, {0x9B6F, 0x10},
    {0x9B71, 0xC8}, {0x9B73, 0x32}, {0x9B75, 0x04}, {0x9B76, 0x0A},
    {0x9BB0, 0x20}, {0x9BB1, 0x20}, {0x9BB2, 0x20}, {0x9BC6, 0x6E},
    {0x9BC7, 0x6E}, {0x9BC8, 0x6E}, {0x9BCC, 0x20}, {0x9BCD, 0x20},
    {0x9BCE, 0x20}, {0x9C01, 0x10}, {0x9C03, 0x1D}, {0x9C05, 0x20},
    {0x9C13, 0x10}, {0x9C15, 0x10}, {0x9C17, 0x10}, {0x9C19, 0x04},
    {0x9C1B, 0x67}, {0x9C1D, 0x80}, {0x9C1F, 0x0A}, {0x9C21, 0x29},
    {0x9C23, 0x32}, {0x9C27, 0x56}, {0x9C29, 0x60}, {0x9C39, 0x67},
    {0x9C3B, 0x80}, {0x9C3D, 0x80}, {0x9C3F, 0x80}, {0x9C41, 0x80},
    {0x9C55, 0xC8}, {0x9C57, 0xC8}, {0x9C59, 0xC8}, {0x9C87, 0x48},
    {0x9C89, 0x48}, {0x9C8B, 0x48}, {0x9CB0, 0x20}, {0x9CB1, 0x20},
    {0x9CB2, 0x20}, {0x9CC6, 0x6E}, {0x9CC7, 0x6E}, {0x9CC8, 0x6E},
    {0x9D13, 0x10}, {0x9D15, 0x10}, {0x9D17, 0x10}, {0x9D19, 0x04},
    {0x9D1B, 0x67}, {0x9D1F, 0x0A}, {0x9D21, 0x29}, {0x9D23, 0x32},
    {0x9D55, 0xC8}, {0x9D57, 0xC8}, {0x9D59, 0xC8}, {0x9D91, 0x20},
    {0x9D93, 0x20}, {0x9D95, 0x20}, {0x9E01, 0x10}, {0x9E03, 0x1D},
    {0x9E13, 0x10}, {0x9E15, 0x10}, {0x9E17, 0x10}, {0x9E19, 0x04},
    {0x9E1B, 0x67}, {0x9E1D, 0x80}, {0x9E1F, 0x0A}, {0x9E21, 0x29},
    {0x9E23, 0x32}, {0x9E25, 0x30}, {0x9E27, 0x56}, {0x9E29, 0x60},
    {0x9E39, 0x67}, {0x9E3B, 0x80}, {0x9E3D, 0x80}, {0x9E3F, 0x80},
    {0x9E41, 0x80}, {0x9E55, 0xC8}, {0x9E57, 0xC8}, {0x9E59, 0xC8},
    {0x9E91, 0x20}, {0x9E93, 0x20}, {0x9E95, 0x20}, {0x9F8F, 0xA0},
    {0xA027, 0x67}, {0xA029, 0x80}, {0xA02D, 0x67}, {0xA02F, 0x80},
    {0xA031, 0x80}, {0xA033, 0x80}, {0xA035, 0x80}, {0xA037, 0x80},
    {0xA039, 0x80}, {0xA03B, 0x80}, {0xA067, 0x20}, {0xA068, 0x20},
    {0xA069, 0x20}, {0xA071, 0x48}, {0xA073, 0x48}, {0xA075, 0x48},
    {0xA08F, 0xA0}, {0xA091, 0x3A}, {0xA093, 0x3A}, {0xA095, 0x0A},
    {0xA097, 0x0A}, {0xA099, 0x0A}, {0x9012, 0x03}, {0x9098, 0x1A},
    {0x9099, 0x04}, {0x909A, 0x20}, {0x909B, 0x20}, {0x909C, 0x13},
    {0x909D, 0x13}, {0xA716, 0x13}, {0xA801, 0x08}, {0xA803, 0x0C},
    {0xA805, 0x10}, {0xA806, 0x00}, {0xA807, 0x18}, {0xA808, 0x00},
    {0xA809, 0x20}, {0xA80A, 0x00}, {0xA80B, 0x30}, {0xA80C, 0x00},
    {0xA80D, 0x40}, {0xA80E, 0x00}, {0xA80F, 0x60}, {0xA810, 0x00},
    {0xA811, 0x80}, {0xA812, 0x00}, {0xA813, 0xC0}, {0xA814, 0x01},
    {0xA815, 0x00}, {0xA816, 0x01}, {0xA817, 0x80}, {0xA818, 0x02},
    {0xA819, 0x00}, {0xA81A, 0x03}, {0xA81B, 0x00}, {0xA81C, 0x03},
    {0xA81D, 0xAC}, {0xA838, 0x03}, {0xA83C, 0x28}, {0xA83D, 0x5F},
    {0xA881, 0x08}, {0xA883, 0x0C}, {0xA885, 0x10}, {0xA886, 0x00},
    {0xA887, 0x18}, {0xA888, 0x00}, {0xA889, 0x20}, {0xA88A, 0x00},
    {0xA88B, 0x30}, {0xA88C, 0x00}, {0xA88D, 0x40}, {0xA88E, 0x00},
    {0xA88F, 0x60}, {0xA890, 0x00}, {0xA891, 0x80}, {0xA892, 0x00},
    {0xA893, 0xC0}, {0xA894, 0x01}, {0xA895, 0x00}, {0xA896, 0x01},
    {0xA897, 0x80}, {0xA898, 0x02}, {0xA899, 0x00}, {0xA89A, 0x03},
    {0xA89B, 0x00}, {0xA89C, 0x03}, {0xA89D, 0xAC}, {0xA8B8, 0x03},
    {0xA8BB, 0x13}, {0xA8BC, 0x28}, {0xA8BD, 0x25}, {0xA8BE, 0x1D},
    {0xA8C0, 0x3A}, {0xA8C1, 0xE0}, {0xB24F, 0x80}, {0x3198, 0x0F},
    {0x31A0, 0x04}, {0x31A1, 0x03}, {0x31A2, 0x02}, {0x31A3, 0x01},
    {0x31A8, 0x18}, {0x822C, 0x01}, {0x8239, 0x01}, {0x9503, 0x07},
    {0x9504, 0x07}, {0x9505, 0x07}, {0x9506, 0x00}, {0x9507, 0x00},
    {0x9508, 0x00}, {0x9526, 0x18}, {0x9527, 0x18}, {0x9528, 0x18},
    {0x8858, 0x00}, {0x6B42, 0x40}, {0x6B46, 0x00}, {0x6B47, 0x4B},
    {0x6B4A, 0x00}, {0x6B4B, 0x4B}, {0x6B4E, 0x00}, {0x6B4F, 0x4B},
    {0x6B44, 0x00}, {0x6B45, 0x8C}, {0x6B48, 0x00}, {0x6B49, 0x8C},
    {0x6B4C, 0x00}, {0x6B4D, 0x8C}, {0x5041, 0x00},
};

static const SENSOR_REG_T imx230_5344x4016_setting[] = {
    {0x0114, 0x03}, {0x0220, 0x00}, {0x0221, 0x11}, {0x0222, 0x01},
    {0x0340, 0x10}, {0x0341, 0x2C}, {0x0342, 0x17}, {0x0343, 0x88},
    {0x0344, 0x00}, {0x0345, 0x00}, {0x0346, 0x00}, {0x0347, 0x00},
    {0x0348, 0x14}, {0x0349, 0xDF}, {0x034A, 0x0F}, {0x034B, 0xAF},
    {0x0381, 0x01}, {0x0383, 0x01}, {0x0385, 0x01}, {0x0387, 0x01},
    {0x0900, 0x00}, {0x0901, 0x11}, {0x0902, 0x00}, {0x3000, 0x74},
    {0x3001, 0x00}, {0x305C, 0x11}, {0x0112, 0x0A}, {0x0113, 0x0A},
    {0x034C, 0x14}, {0x034D, 0xE0}, {0x034E, 0x0F}, {0x034F, 0xB0},
    {0x0401, 0x00}, {0x0404, 0x00}, {0x0405, 0x10}, {0x0408, 0x00},
    {0x0409, 0x00}, {0x040A, 0x00}, {0x040B, 0x00}, {0x040C, 0x14},
    {0x040D, 0xE0}, {0x040E, 0x0F}, {0x040F, 0xB0}, {0x0301, 0x04},
    {0x0303, 0x02}, {0x0305, 0x04}, {0x0306, 0x00}, {0x0307, 0xC8},
    {0x0309, 0x0A}, {0x030B, 0x01}, {0x030D, 0x0F}, {0x030E, 0x03},
    {0x030F, 0x77}, {0x0310, 0x01}, {0x0820, 0x16}, {0x0821, 0x2C},
    {0x0822, 0xCC}, {0x0823, 0xCC}, {0x0202, 0x10}, {0x0203, 0x22},
    {0x0224, 0x01}, {0x0225, 0xF4}, {0x0204, 0x00}, {0x0205, 0x00},
    {0x0216, 0x00}, {0x0217, 0x00}, {0x020E, 0x01}, {0x020F, 0x00},
    {0x0210, 0x01}, {0x0211, 0x00}, {0x0212, 0x01}, {0x0213, 0x00},
    {0x0214, 0x01}, {0x0215, 0x00}, {0x3006, 0x01}, {0x3007, 0x02},
    {0x31E0, 0x03}, {0x31E1, 0xFF}, {0x31E4, 0x02}, {0x3A22, 0x20},
    {0x3A23, 0x14}, {0x3A24, 0xE0}, {0x3A25, 0x0F}, {0x3A26, 0xB0},
    {0x3A2F, 0x00}, {0x3A30, 0x00}, {0x3A31, 0x00}, {0x3A32, 0x00},
    {0x3A33, 0x14}, {0x3A34, 0xDF}, {0x3A35, 0x0F}, {0x3A36, 0xAF},
    {0x3A37, 0x00}, {0x3A38, 0x00}, {0x3A39, 0x00}, {0x3A21, 0x00},
    {0x3011, 0x00}, {0x3013, 0x01},
};
static const SENSOR_REG_T imx230_4272x3212_setting[] = {
    /*4Lane
    reg_A4
    4272x3212 (4:3)
    H: 4272
    V: 3212
    Mode Setting
            Address value*/
    {0x0114, 0x03}, {0x0220, 0x00}, {0x0221, 0x11},
    {0x0222, 0x01}, {0x0340, 0x10}, {0x0341, 0x0A},
    {0x0342, 0x17}, {0x0343, 0x88}, {0x0344, 0x00},
    {0x0345, 0x00}, {0x0346, 0x00}, {0x0347, 0x00},
    {0x0348, 0x14}, {0x0349, 0xDF}, {0x034A, 0x0F},
    {0x034B, 0xAF}, {0x0381, 0x01}, {0x0383, 0x01},
    {0x0385, 0x01}, {0x0387, 0x01}, {0x0900, 0x00},
    {0x0901, 0x11}, {0x0902, 0x00}, {0x3000, 0x74},
    {0x3001, 0x00}, {0x305C, 0x11}, // Output Size Setting
    {0x0112, 0x0A}, {0x0113, 0x0A}, {0x034C, 0x10},
    {0x034D, 0xB0}, {0x034E, 0x0C}, {0x034F, 0x8C},
    {0x0401, 0x02}, {0x0404, 0x00}, {0x0405, 0x14},
    {0x0408, 0x00}, {0x0409, 0x02}, {0x040A, 0x00},
    {0x040B, 0x00}, {0x040C, 0x14}, {0x040D, 0xDE},
    {0x040E, 0x0F}, {0x040F, 0xB0}, // Clock Setting
    {0x0301, 0x04}, {0x0303, 0x02}, {0x0305, 0x04},
    {0x0306, 0x00}, {0x0307, 0xC6}, {0x0309, 0x0A},
    {0x030B, 0x01}, {0x030D, 0x0F}, {0x030E, 0x02},
    {0x030F, 0xC3}, {0x0310, 0x01}, // Data Rate Setting
    {0x0820, 0x11}, {0x0821, 0xAC}, {0x0822, 0xCC},
    {0x0823, 0xCC}, // Integration Time Setting
    {0x0202, 0x10}, {0x0203, 0x00}, {0x0224, 0x01},
    {0x0225, 0xF4}, // Gain Setting
    {0x0204, 0x00}, {0x0205, 0x00}, {0x0216, 0x00},
    {0x0217, 0x00}, {0x020E, 0x01}, {0x020F, 0x00},
    {0x0210, 0x01}, {0x0211, 0x00}, {0x0212, 0x01},
    {0x0213, 0x00}, {0x0214, 0x01}, {0x0215, 0x00}, // HDR Setting
    {0x3006, 0x01}, {0x3007, 0x02}, {0x31E0, 0x03},
    {0x31E1, 0xFF}, {0x31E4, 0x02}, // DPC2D Setting
    {0x3A22, 0x20}, {0x3A23, 0x14}, {0x3A24, 0xE0},
    {0x3A25, 0x0F}, {0x3A26, 0xB0}, {0x3A2F, 0x00},
    {0x3A30, 0x00}, {0x3A31, 0x00}, {0x3A32, 0x00},
    {0x3A33, 0x14}, {0x3A34, 0xDF}, {0x3A35, 0x0F},
    {0x3A36, 0xAF}, {0x3A37, 0x00}, {0x3A38, 0x00},
    {0x3A39, 0x00}, // LSC Setting
    {0x3A21, 0x00}, // Stats Setting
    {0x3011, 0x00}, {0x3013, 0x01},

};

static const SENSOR_REG_T imx230_4160x3120_setting[] = {
    /*4Lane
    reg_A4
    4160x3120 (4:3)
    H: 4160
    V: 3120
    Mode Setting
            Address	value*/
    {0x0114, 0x03}, {0x0220, 0x00}, {0x0221, 0x11},
    {0x0222, 0x01}, {0x0340, 0x10}, {0x0341, 0x0C},
    {0x0342, 0x17}, {0x0343, 0x88}, {0x0344, 0x00},
    {0x0345, 0x00}, {0x0346, 0x00}, {0x0347, 0x00},
    {0x0348, 0x14}, {0x0349, 0xDF}, {0x034A, 0x0F},
    {0x034B, 0xAF}, {0x0381, 0x01}, {0x0383, 0x01},
    {0x0385, 0x01}, {0x0387, 0x01}, {0x0900, 0x00},
    {0x0901, 0x11}, {0x0902, 0x00}, {0x3000, 0x74},
    {0x3001, 0x00}, {0x305C, 0x11}, // Output Size Setting
    {0x0112, 0x0A}, {0x0113, 0x0A}, {0x034C, 0x10},
    {0x034D, 0x40}, {0x034E, 0x0C}, {0x034F, 0x30},
    {0x0401, 0x02}, {0x0404, 0x00}, {0x0405, 0x14},
    {0x0408, 0x00}, {0x0409, 0x48}, {0x040A, 0x00},
    {0x040B, 0x3A}, {0x040C, 0x14}, {0x040D, 0x52},
    {0x040E, 0x0F}, {0x040F, 0x3E}, // Clock Setting
    {0x0301, 0x04}, {0x0303, 0x02}, {0x0305, 0x04},
    {0x0306, 0x00}, {0x0307, 0xC6}, {0x0309, 0x0A},
    {0x030B, 0x01}, {0x030D, 0x0F}, {0x030E, 0x02},
    {0x030F, 0xB1}, {0x0310, 0x01}, // Data Rate Setting
    {0x0820, 0x11}, {0x0821, 0x39}, {0x0822, 0x99},
    {0x0823, 0x99}, // Integration Time Setting
    {0x0202, 0x10}, {0x0203, 0x02}, {0x0224, 0x01},
    {0x0225, 0xF4}, // Gain Setting
    {0x0204, 0x00}, {0x0205, 0x00}, {0x0216, 0x00},
    {0x0217, 0x00}, {0x020E, 0x01}, {0x020F, 0x00},
    {0x0210, 0x01}, {0x0211, 0x00}, {0x0212, 0x01},
    {0x0213, 0x00}, {0x0214, 0x01}, {0x0215, 0x00}, // HDR Setting
    {0x3006, 0x01}, {0x3007, 0x02}, {0x31E0, 0x03},
    {0x31E1, 0xFF}, {0x31E4, 0x02}, // DPC2D Setting
    {0x3A22, 0x20}, {0x3A23, 0x14}, {0x3A24, 0xE0},
    {0x3A25, 0x0F}, {0x3A26, 0xB0}, {0x3A2F, 0x00},
    {0x3A30, 0x00}, {0x3A31, 0x00}, {0x3A32, 0x00},
    {0x3A33, 0x14}, {0x3A34, 0xDF}, {0x3A35, 0x0F},
    {0x3A36, 0xAF}, {0x3A37, 0x00}, {0x3A38, 0x00},
    {0x3A39, 0x00}, // LSC Setting
    {0x3A21, 0x00}, // Stats Setting
    {0x3011, 0x00}, {0x3013, 0x01},

};

static const SENSOR_REG_T imx230_4272x2404_setting[] = {
    /*4Lane
    reg_A4
    4K2K 30fps
    H: 4272
    V: 2404
    Mode Setting*/
    {0x0114, 0x03}, {0x0220, 0x00}, {0x0221, 0x11},
    {0x0222, 0x01}, {0x0340, 0x0C}, {0x0341, 0x20},
    {0x0342, 0x17}, {0x0343, 0x88}, {0x0344, 0x00},
    {0x0345, 0x00}, {0x0346, 0x01}, {0x0347, 0xF8},
    {0x0348, 0x14}, {0x0349, 0xDF}, {0x034A, 0x0D},
    {0x034B, 0xB7}, {0x0381, 0x01}, {0x0383, 0x01},
    {0x0385, 0x01}, {0x0387, 0x01}, {0x0900, 0x00},
    {0x0901, 0x11}, {0x0902, 0x00}, {0x3000, 0x74},
    {0x3001, 0x00}, {0x305C, 0x11}, // Output Size Setting
    {0x0112, 0x0A}, {0x0113, 0x0A}, {0x034C, 0x10},
    {0x034D, 0xB0}, {0x034E, 0x09}, {0x034F, 0x64},
    {0x0401, 0x02}, {0x0404, 0x00}, {0x0405, 0x14},
    {0x0408, 0x00}, {0x0409, 0x02}, {0x040A, 0x00},
    {0x040B, 0x02}, {0x040C, 0x14}, {0x040D, 0xDE},
    {0x040E, 0x0B}, {0x040F, 0xBE}, // Clock Setting
    {0x0301, 0x04}, {0x0303, 0x02}, {0x0305, 0x04},
    {0x0306, 0x00}, {0x0307, 0xBB}, {0x0309, 0x0A},
    {0x030B, 0x01}, {0x030D, 0x0F}, {0x030E, 0x02},
    {0x030F, 0xAF}, {0x0310, 0x01}, // Data Rate Setting
    {0x0820, 0x11}, {0x0821, 0x2C}, {0x0822, 0xCC},
    {0x0823, 0xCC}, // Integration Time Setting
    {0x0202, 0x0C}, {0x0203, 0x16}, {0x0224, 0x01},
    {0x0225, 0xF4}, // Gain Setting
    {0x0204, 0x00}, {0x0205, 0x00}, {0x0216, 0x00},
    {0x0217, 0x00}, {0x020E, 0x01}, {0x020F, 0x00},
    {0x0210, 0x01}, {0x0211, 0x00}, {0x0212, 0x01},
    {0x0213, 0x00}, {0x0214, 0x01}, {0x0215, 0x00}, // HDR Setting
    {0x3006, 0x01}, {0x3007, 0x02}, {0x31E0, 0x03},
    {0x31E1, 0xFF}, {0x31E4, 0x02}, // DPC2D Setting
    {0x3A22, 0x20}, {0x3A23, 0x14}, {0x3A24, 0xE0},
    {0x3A25, 0x0B}, {0x3A26, 0xC0}, {0x3A2F, 0x00},
    {0x3A30, 0x00}, {0x3A31, 0x01}, {0x3A32, 0xF8},
    {0x3A33, 0x14}, {0x3A34, 0xDF}, {0x3A35, 0x0D},
    {0x3A36, 0xB7}, {0x3A37, 0x00}, {0x3A38, 0x00},
    {0x3A39, 0x00}, // LSC Setting
    {0x3A21, 0x00}, // Stats Setting
    {0x3011, 0x00}, {0x3013, 0x01},

};

static const SENSOR_REG_T imx230_2672x2008_setting_new[] = {
    /*4Lane
    reg_B4
    2672x2008 (4:3)
    H: 2672
    V: 2008
    Mode Setting
            Address value*/
    {0x0114, 0x03}, {0x0220, 0x00}, {0x0221, 0x11},
    {0x0222, 0x01}, {0x0340, 0x0B}, {0x0341, 0x7A},
    {0x0342, 0x17}, {0x0343, 0x88}, {0x0344, 0x00},
    {0x0345, 0x00}, {0x0346, 0x00}, {0x0347, 0x00},
    {0x0348, 0x14}, {0x0349, 0xDF}, {0x034A, 0x0F},
    {0x034B, 0xAF}, {0x0381, 0x01}, {0x0383, 0x01},
    {0x0385, 0x01}, {0x0387, 0x01}, {0x0900, 0x01},
    {0x0901, 0x22}, {0x0902, 0x02}, {0x3000, 0x74},
    {0x3001, 0x00}, {0x305C, 0x11}, // Output Size Setting
    {0x0112, 0x0A}, {0x0113, 0x0A}, {0x034C, 0x0A},
    {0x034D, 0x70}, {0x034E, 0x07}, {0x034F, 0xD8},
    {0x0401, 0x00}, {0x0404, 0x00}, {0x0405, 0x10},
    {0x0408, 0x00}, {0x0409, 0x00}, {0x040A, 0x00},
    {0x040B, 0x00}, {0x040C, 0x0A}, {0x040D, 0x70},
    {0x040E, 0x07}, {0x040F, 0xD8}, // Clock Setting
    {0x0301, 0x04}, {0x0303, 0x02}, {0x0305, 0x04},
    {0x0306, 0x00}, {0x0307, 0xB1}, {0x0309, 0x0A},
    {0x030B, 0x01}, {0x030D, 0x0F}, {0x030E, 0x03},
    {0x030F, 0xA9}, {0x0310, 0x01}, // Data Rate Setting
    {0x0820, 0x17}, {0x0821, 0x6C}, {0x0822, 0xCC},
    {0x0823, 0xCC}, // Integration Time Setting
    {0x0202, 0x0B}, {0x0203, 0x70}, {0x0224, 0x01},
    {0x0225, 0xF4}, // Gain Setting
    {0x0204, 0x00}, {0x0205, 0x00}, {0x0216, 0x00},
    {0x0217, 0x00}, {0x020E, 0x01}, {0x020F, 0x00},
    {0x0210, 0x01}, {0x0211, 0x00}, {0x0212, 0x01},
    {0x0213, 0x00}, {0x0214, 0x01}, {0x0215, 0x00}, // HDR Setting
    {0x3006, 0x01}, {0x3007, 0x02}, {0x31E0, 0x03},
    {0x31E1, 0xFF}, {0x31E4, 0x02}, // DPC2D Setting
    {0x3A22, 0x20}, {0x3A23, 0x14}, {0x3A24, 0xE0},
    {0x3A25, 0x07}, {0x3A26, 0xD8}, {0x3A2F, 0x00},
    {0x3A30, 0x00}, {0x3A31, 0x00}, {0x3A32, 0x00},
    {0x3A33, 0x14}, {0x3A34, 0xDF}, {0x3A35, 0x0F},
    {0x3A36, 0xAF}, {0x3A37, 0x00}, {0x3A38, 0x01},
    {0x3A39, 0x00}, // LSC Setting
    {0x3A21, 0x00}, {0x3011, 0x00}, {0x3013, 0x01},
};

static const SENSOR_REG_T imx230_2672x2008_setting[] = {
    {0x0114, 0x03},
    {0x0220, 0x00},
    {0x0221, 0x11},
    {0x0222, 0x01},
    {0x0340, 0x09},
    {0x0341, 0x10},
    {0x0342, 0x17},
    {0x0343, 0x88},
    {0x0344, 0x00},
    {0x0345, 0x00},
    {0x0346, 0x00},
    {0x0347, 0x00},
    {0x0348, 0x14},
    {0x0349, 0xDF},
    {0x034A, 0x0F},
    {0x034B, 0xAF},
    {0x0381, 0x01},
    {0x0383, 0x01},
    {0x0385, 0x01},
    {0x0387, 0x01},
    {0x0900, 0x01},
    {0x0901, 0x22},
    {0x0902, 0x02},
    {0x3000, 0x74},
    {0x3001, 0x00},
    {0x305C, 0x11},
    {0x0112, 0x0A},
    {0x0113, 0x0A},
    {0x034C, 0x0A},
    {0x034D, 0x70},
    {0x034E, 0x07},
    {0x034F, 0xD8},
    {0x0401, 0x00},
    {0x0404, 0x00},
    {0x0405, 0x10},
    {0x0408, 0x00},
    {0x0409, 0x00},
    {0x040A, 0x00},
    {0x040B, 0x00},
    {0x040C, 0x0A},
    {0x040D, 0x70},
    {0x040E, 0x07},
    {0x040F, 0xD8},
    {0x0301, 0x04},
    {0x0303, 0x02},
    {0x0305, 0x04},
    {0x0306, 0x00},
    {0x0307, 0x8C},
    {0x0309, 0x0A},
    {0x030B, 0x01},
    {0x030D, 0x0F},
    {0x030E, 0x01},
    {0x030F, 0xF4},
    {0x0310, 0x01},
    {0x0820, 0x0C},
    {0x0821, 0x80},
    {0x0822, 0x00},
    {0x0823, 0x00},
    {0x0202, 0x09},
    {0x0203, 0x06},
    {0x0224, 0x01},
    {0x0225, 0xF4},
    {0x0204, 0x00},
    {0x0205, 0x00},
    {0x0216, 0x00},
    {0x0217, 0x00},
    {0x020E, 0x01},
    {0x020F, 0x00},
    {0x0210, 0x01},
    {0x0211, 0x00},
    {0x0212, 0x01},
    {0x0213, 0x00},
    {0x0214, 0x01},
    {0x0215, 0x00},
    {0x3006, 0x01},
    {0x3007, 0x02},
    {0x31E0, 0x03},
    {0x31E1, 0xFF},
    {0x31E4, 0x02},
    {0x3A22, 0x20},
    {0x3A23, 0x14},
    {0x3A24, 0xE0},
    {0x3A25, 0x07},
    {0x3A26, 0xD8},
    {0x3A2F, 0x00},
    {0x3A30, 0x00},
    {0x3A31, 0x00},
    {0x3A32, 0x00},
    {0x3A33, 0x14},
    {0x3A34, 0xDF},
    {0x3A35, 0x0F},
    {0x3A36, 0xAF},
    {0x3A37, 0x00},
    {0x3A38, 0x01},
    {0x3A39, 0x00},
    {0x3A21, 0x00},
    {0x3011, 0x00},
    {0x3013, 0x01},
#ifdef CONFIG_CAMERA_DUAL_SYNC
    /*for sensor sync start*/
    {0x440C, 0x00}, // Lo-Activate
    {0x440D, 0x07}, // pulse width(2000cycle)
    // V-sync output pin settings: FSTROBE setting
    {0x4073, 0xFF},
    {0x5ED0, 0x00},
    {0x5E69, 0xFF},
    {0x5E6A, 0x00},
    {0x5E6B, 0x00},
    {0x5E70, 0x02},
/*for sensor sync end*/
#endif
};

/**
 line time: 10us
 MipiData Rate= 400Mbps/Lane
 Frame length lines = 828 line
 */
static const SENSOR_REG_T imx230_1280x720_setting[] = {
    {0x0114, 0x03}, {0x0220, 0x00}, {0x0221, 0x11}, {0x0222, 0x01},
    {0x0340, 0x03}, {0x0341, 0x3C}, {0x0342, 0x17}, {0x0343, 0x88},
    {0x0344, 0x00}, {0x0345, 0x00}, {0x0346, 0x02}, {0x0347, 0x38},
    {0x0348, 0x14}, {0x0349, 0xDF}, {0x034A, 0x0D}, {0x034B, 0x77},
    {0x0381, 0x01}, {0x0383, 0x01}, {0x0385, 0x01}, {0x0387, 0x01},
    {0x0900, 0x01}, {0x0901, 0x44}, {0x0902, 0x02}, {0x3000, 0x74},
    {0x3001, 0x00}, {0x305C, 0x11}, {0x0112, 0x0A}, {0x0113, 0x0A},
    {0x034C, 0x05}, {0x034D, 0x00}, {0x034E, 0x02}, {0x034F, 0xD0},
    {0x0401, 0x02}, {0x0404, 0x00}, {0x0405, 0x10}, {0x0408, 0x00},
    {0x0409, 0x1C}, {0x040A, 0x00}, {0x040B, 0x00}, {0x040C, 0x05},
    {0x040D, 0x00}, {0x040E, 0x02}, {0x040F, 0xD0}, {0x0301, 0x04},
    {0x0303, 0x02}, {0x0305, 0x04}, {0x0306, 0x00}, {0x0307, 0xC8},
    {0x0309, 0x0A}, {0x030B, 0x01}, {0x030D, 0x0F}, {0x030E, 0x00},
    {0x030F, 0xFA}, {0x0310, 0x01}, {0x0820, 0x06}, {0x0821, 0x40},
    {0x0822, 0x00}, {0x0823, 0x00}, {0x0202, 0x03}, {0x0203, 0x32},
    {0x0224, 0x01}, {0x0225, 0xF4}, {0x0204, 0x00}, {0x0205, 0x00},
    {0x0216, 0x00}, {0x0217, 0x00}, {0x020E, 0x01}, {0x020F, 0x00},
    {0x0210, 0x01}, {0x0211, 0x00}, {0x0212, 0x01}, {0x0213, 0x00},
    {0x0214, 0x01}, {0x0215, 0x00}, {0x3006, 0x01}, {0x3007, 0x02},
    {0x31E0, 0x03}, {0x31E1, 0xFF}, {0x31E4, 0x02}, {0x3A22, 0x20},
    {0x3A23, 0x14}, {0x3A24, 0xE0}, {0x3A25, 0x02}, {0x3A26, 0xD0},
    {0x3A2F, 0x00}, {0x3A30, 0x00}, {0x3A31, 0x02}, {0x3A32, 0x38},
    {0x3A33, 0x14}, {0x3A34, 0xDF}, {0x3A35, 0x0D}, {0x3A36, 0x77},
    {0x3A37, 0x00}, {0x3A38, 0x02}, {0x3A39, 0x00}, {0x3A21, 0x00},
    {0x3011, 0x00}, {0x3013, 0x01}};

static struct sensor_res_tab_info s_imx230_resolution_tab_raw[VENDOR_NUM] = {
    {
      .module_id = MODULE_SUNNY,
      .reg_tab = {
        {ADDR_AND_LEN_OF_ARRAY(imx230_init_setting), PNULL, 0,
        .width = 0, .height = 0,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},

        {ADDR_AND_LEN_OF_ARRAY(imx230_1280x720_setting), PNULL, 0,
        .width = 1280, .height = 720,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},

        {ADDR_AND_LEN_OF_ARRAY(imx230_2672x2008_setting), PNULL, 0,
        .width = 2672, .height = 2008,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},

        /*{ADDR_AND_LEN_OF_ARRAY(imx230_2672x2008_setting_new), PNULL, 0,
        .width = 2672, .height = 2008,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},*/

        {ADDR_AND_LEN_OF_ARRAY(imx230_4272x2404_setting), PNULL, 0,
        .width = 4272, .height = 2404,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},

        {ADDR_AND_LEN_OF_ARRAY(imx230_4272x3212_setting), PNULL, 0,
        .width = 4272, .height = 3212,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},

        /*{ADDR_AND_LEN_OF_ARRAY(imx230_5344x4016_setting), PNULL, 0,
        .width = 5344, .height = 4016,
        .xclk_to_sensor = EX_MCLK, .image_format = SENSOR_IMAGE_FORMAT_RAW},*/
    }}
/*If there are multiple modules,please add here*/
};

static SENSOR_TRIM_T s_imx230_resolution_trim_tab[VENDOR_NUM] = {
    {
     .module_id = MODULE_SUNNY,
     .trim_info = {
       {0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},
       { .trim_start_x = 0, .trim_start_y = 0,
         .trim_width = 1280, .trim_height = 720,
         .line_time = 10188,.bps_per_lane = PREVIEW_MIPI_PER_LANE_BPS,
         .frame_line = 828,
         .scaler_trim = {.x = 0, .y = 0, .w = 1280, .h = 720}},

       { .trim_start_x = 0, .trim_start_y = 0,
         .trim_width = 2672, .trim_height = 2008,
         .line_time = 14343,.bps_per_lane = PREVIEW_MIPI_PER_LANE_BPS,
         .frame_line = 2320,
         .scaler_trim = {.x = 0, .y = 0, .w = 2672, .h = 2008}},

       { .trim_start_x = 0, .trim_start_y = 0,
         .trim_width = 4272, .trim_height = 2404,
         .line_time = 10739,.bps_per_lane = 1099,
         .frame_line = 3104,
         .scaler_trim = {.x = 0, .y = 0, .w = 4272, .h = 2404}},

       { .trim_start_x = 0, .trim_start_y = 0,
         .trim_width = 4272, .trim_height = 3212,
         .line_time = 10144,.bps_per_lane = 1131,
         .frame_line = 4106,
         .scaler_trim = {.x = 0, .y = 0, .w = 4272, .h = 3212}},

       /*{ .trim_start_x = 0, .trim_start_y = 0,
         .trim_width = 5344, .trim_height = 4016,
         .line_time = 10040,.bps_per_lane = SNAPSHOT_MIPI_PER_LANE_BPS,
         .frame_line = 4140,
         .scaler_trim = {.x = 0, .y = 0, .w = 5344, .h = 4016}},*/

    }}
    /*If there are multiple modules,please add here*/
};

static const SENSOR_REG_T
    s_imx230_preview_size_video_tab[SENSOR_VIDEO_MODE_MAX][1] = {
    /*video mode 0: ?fps */
    {{0xffff, 0xff}},
    /* video mode 1:?fps */
    {{0xffff, 0xff}},
    /* video mode 2:?fps */
    {{0xffff, 0xff}},
    /* video mode 3:?fps */
    {{0xffff, 0xff}}};

static const SENSOR_REG_T
    s_imx230_capture_size_video_tab[SENSOR_VIDEO_MODE_MAX][1] = {
    /*video mode 0: ?fps */
    {{0xffff, 0xff}},
    /* video mode 1:?fps */
    {{0xffff, 0xff}},
    /* video mode 2:?fps */
    {{0xffff, 0xff}},
    /* video mode 3:?fps */
    {{0xffff, 0xff}}};

static SENSOR_VIDEO_INFO_T s_imx230_video_info[SENSOR_MODE_MAX] = {
    {{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, PNULL},
    {{{30, 30, 270, 90}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     (SENSOR_REG_T **)s_imx230_preview_size_video_tab},
    {{{2, 5, 338, 1000}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     (SENSOR_REG_T **)s_imx230_capture_size_video_tab},
};

static SENSOR_STATIC_INFO_T s_imx230_static_info[VENDOR_NUM] = {
    {.module_id = MODULE_TRULY,
     .static_info = {
        .f_num = 220,
        .focal_length = 462,
        .max_fps = 0,
        .max_adgain = 8 * 16,
        .ois_supported = 0,
        .pdaf_supported = 0,
        .exp_valid_frame_num = 1,
        .clamp_level = 64,
        .adgain_valid_frame_num = 1,
        .fov_info = {{5.985f, 4.498f}, 4.75f}}
    }
};

static SENSOR_MODE_FPS_INFO_T s_imx230_mode_fps_info[VENDOR_NUM] = {
    {.module_id = MODULE_TRULY,
        {.is_init = 0,
          {{SENSOR_MODE_COMMON_INIT, 0, 1, 0, 0},
           {SENSOR_MODE_PREVIEW_ONE, 0, 1, 0, 0},
           {SENSOR_MODE_SNAPSHOT_ONE_FIRST, 0, 1, 0, 0},
           {SENSOR_MODE_SNAPSHOT_ONE_SECOND, 0, 1, 0, 0},
           {SENSOR_MODE_SNAPSHOT_ONE_THIRD, 0, 1, 0, 0},
           {SENSOR_MODE_PREVIEW_TWO, 0, 1, 0, 0},
           {SENSOR_MODE_SNAPSHOT_TWO_FIRST, 0, 1, 0, 0},
           {SENSOR_MODE_SNAPSHOT_TWO_SECOND, 0, 1, 0, 0},
           {SENSOR_MODE_SNAPSHOT_TWO_THIRD, 0, 1, 0, 0}}}
    }
};

static struct sensor_module_info s_imx230_module_info_tab[VENDOR_NUM] = {
    {.module_id = MODULE_TRULY,
     .module_info = {
        .major_i2c_addr = I2C_SLAVE_ADDR >> 1,
        .minor_i2c_addr = I2C_SLAVE_ADDR >> 1,

        .reg_addr_value_bits = SENSOR_I2C_REG_16BIT | SENSOR_I2C_VAL_8BIT |
                               SENSOR_I2C_FREQ_400,

        .avdd_val = SENSOR_AVDD_2500MV,
        .iovdd_val = SENSOR_AVDD_1800MV,
        .dvdd_val = SENSOR_AVDD_1200MV,

        .image_pattern = SENSOR_IMAGE_PATTERN_RAWRGB_R,

        .preview_skip_num = 1,
        .capture_skip_num = 1,
        .flash_capture_skip_num = 6,
        .mipi_cap_skip_num = 0,
        .preview_deci_num = 0,
        .video_preview_deci_num = 0,

        .sensor_interface = {
            .type = SENSOR_INTERFACE_TYPE_CSI2,
            .bus_width = LANE_NUM,
            .pixel_width = RAW_BITS,
            /*0:mipi_raw,1:normal_raw*/
            .is_loose = 0,
        },

        .change_setting_skip_num = 1,
        .horizontal_view_angle = 35,
        .vertical_view_angle = 35
    }}
};

static struct sensor_ic_ops s_imx230_ops_tab;
struct sensor_raw_info *s_imx230_mipi_raw_info_ptr = &s_imx230_mipi_raw_info;

SENSOR_INFO_T g_imx230_mipi_raw_info = {
    .hw_signal_polarity = SENSOR_HW_SIGNAL_PCLK_P | SENSOR_HW_SIGNAL_VSYNC_P |
                          SENSOR_HW_SIGNAL_HSYNC_P,
    .environment_mode = SENSOR_ENVIROMENT_NORMAL | SENSOR_ENVIROMENT_NIGHT,
    .image_effect = SENSOR_IMAGE_EFFECT_NORMAL | SENSOR_IMAGE_EFFECT_BLACKWHITE |
                    SENSOR_IMAGE_EFFECT_RED | SENSOR_IMAGE_EFFECT_GREEN |
                    SENSOR_IMAGE_EFFECT_BLUE | SENSOR_IMAGE_EFFECT_YELLOW |
                    SENSOR_IMAGE_EFFECT_NEGATIVE | SENSOR_IMAGE_EFFECT_CANVAS,

    .wb_mode = 0,
    .step_count = 7,

    .reset_pulse_level = SENSOR_LOW_PULSE_RESET,
    .reset_pulse_width = 50,
    .power_down_level = SENSOR_LOW_LEVEL_PWDN,

    .identify_count = 1,
    .identify_code = { {imx230_PID_ADDR, imx230_PID_VALUE}, 
                       {imx230_VER_ADDR, imx230_VER_VALUE}},

    .source_width_max = SNAPSHOT_WIDTH,
    .source_height_max = SNAPSHOT_HEIGHT,
    .name = (cmr_s8 *) SENSOR_NAME,

    .image_format = SENSOR_IMAGE_FORMAT_RAW,

    .resolution_tab_info_ptr = s_imx230_resolution_tab_raw,
    .sns_ops = &s_imx230_ops_tab,

    .raw_info_ptr = &s_imx230_mipi_raw_info_ptr,
    .module_info_tab = s_imx230_module_info_tab,
    .module_info_tab_size = ARRAY_SIZE(s_imx230_module_info_tab),
    .ext_info_ptr = NULL,

    .video_tab_info_ptr = NULL,
    .sensor_version_info = (cmr_s8 *)"imx230v1",
};

#endif