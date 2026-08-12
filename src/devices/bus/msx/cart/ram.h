// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_RAM_H
#define MAME_BUS_MSX_CART_RAM_H

#pragma once

#include "bus/msx/slot/cartridge.h"

void msx_cart_ram_register_options(device_slot_interface &device);

DECLARE_DEVICE_TYPE(MSX_CART_WONDERTANG_MEGARAM, msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_WONDERTANG_MM, msx_cart_interface)

#endif // MAME_BUS_MSX_CART_RAM_H
