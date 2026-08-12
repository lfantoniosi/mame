// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/bw12_dsk.cpp

    bw12 format

*********************************************************************/

#include "formats/bw12_dsk.h"

bw12_format::bw12_format() : upd765_format(formats)
{
}

const char *bw12_format::name() const noexcept
{
	return "bw12";
}

const char *bw12_format::description() const noexcept
{
	return "Bondwell 12/14 disk image";
}

const char *bw12_format::extensions() const noexcept
{
	return "dsk";
}

// A flux capture shows 2:1 sector interleave with gap1=50, gap2=22 and
// gap3=12 (matching the FORMAT TRACK GPL value of 0x0c).
const bw12_format::format bw12_format::formats[] = {
	{ // 180KB BW 12
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, -1, { 0, 9, 1, 10, 2, 11, 3, 12, 4, 13, 5, 14, 6, 15, 7, 16, 8, 17 }, 80, 50, 22, 12
	},
	{ // 360KB BW 14
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 18, 40, 2, 256, {}, -1, { 0, 9, 1, 10, 2, 11, 3, 12, 4, 13, 5, 14, 6, 15, 7, 16, 8, 17 }, 80, 50, 22, 12
	},
	{ // SVI-328
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 17, 40, 1, 256, {}, 0, {}, 80, 50, 22, 80
	},
	{ // SVI-328
		floppy_image::FF_525, floppy_image::DSDD, floppy_image::MFM,
		2000, 17, 40, 2, 256, {}, 0, {}, 80, 50, 22, 80
	},
	{ // Kaypro II
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, 0, {}, 80, 50, 22, 80
	},
	{}
};

const bw12_format FLOPPY_BW12_FORMAT;
