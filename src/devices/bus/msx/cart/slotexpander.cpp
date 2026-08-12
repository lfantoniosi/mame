// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "slotexpander.h"
#include "bus/msx/cart/cartridge.h"
#include "bus/msx/cart/fmpac.h"
#include "bus/msx/cart/franky.h"
#include "bus/msx/cart/ide.h"
#include "bus/msx/cart/ram.h"
#include "bus/msx/cart/slotoptions.h"
#include "machine/input_merger.h"

/*
Emulation of a 4 slot expander for the MSX system.

Some slot expanders have the ability to enable/disable subslots through
jumpers/switches, or adjustable volume per slot. None of that is emulated.

Known slot expanders:
- 8bits4ever Slot x4 - audio mixer with adjustable volume per slot not emulated.
- CIEL Mini Slot Expander - Jumpers and switches not emulated.
- Digital Design Expansor de slots DDX
- ECC Expansion Computer Case - 8 slot version not emulated
- Front Line Slot Expander
- G.DOS Slotexpander
- Hans Oranje slotexpander - Switches not emulated
- Incompel Expansor de slots
- MAD Expander Slot Box MXE-MAIN-A / MXE-MAIN4-A - Switches not emulated
- MK Slotexpander - Switches not emulated
- MSX Club Gouda Slotexpander - Switches not emulated
- Mitsubishi ML-20EB
- Neos EX-4
- Padial LPE-4EXP-V3SC
- Padial LPE-4EXP-V4SC
- Repro Factory My Super Expander 4X
- Sinfox Slotexpander
- Sony HBI-50 - Only expands to 2 slots
- Sunrise 8x SlotExpander - only 4 slots emulated
- Supersoniqs Modulon
- Toshiba HX-E601
- Victor HC-A703E - Only expands to 2 slots?
- Zemina Expansion slot - Only expands to 2 slots

*/

namespace {

class msx_cart_slotexpander_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_slotexpander_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_SLOTEXPANDER, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_cartslot(*this, "cartslot%u", 1)
		, m_irq_out(*this, "irq_out")
		, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"} }
		, m_secondary_slot(0)
	{ }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_config_complete() override;

private:
	template<int Slot> void add_cartslot(machine_config &mconfig);
	u8 secondary_slot_r();
	void secondary_slot_w(u8 data);

	required_device_array<msx_slot_cartridge_device, 4> m_cartslot;
	required_device<input_merger_any_high_device> m_irq_out;
	memory_view m_view[4];
	u8 m_secondary_slot;
};

template<int Slot>
void msx_cart_slotexpander_device::add_cartslot(machine_config &mconfig)
{
	MSX_SLOT_CARTRIDGE(mconfig, m_cartslot[Slot], DERIVED_CLOCK(1, 1));
	m_cartslot[Slot]->option_reset();
	msx_cart(*m_cartslot[Slot], true);
	m_cartslot[Slot]->set_default_option(nullptr);
	m_cartslot[Slot]->set_fixed(false);
	m_cartslot[Slot]->irq_handler().set(m_irq_out, FUNC(input_merger_device::in_w<Slot>));
	if (parent_slot())
	{
		m_cartslot[Slot]->add_route(ALL_OUTPUTS, soundin(), 1.0);
	}
}

void msx_cart_slotexpander_device::device_add_mconfig(machine_config &mconfig)
{
	add_cartslot<0>(mconfig);
	add_cartslot<1>(mconfig);
	add_cartslot<2>(mconfig);
	add_cartslot<3>(mconfig);

	INPUT_MERGER_ANY_HIGH(mconfig, m_irq_out).output_handler().set(*this, FUNC(msx_cart_slotexpander_device::irq_out));
}

void msx_cart_slotexpander_device::device_config_complete()
{
	if (parent_slot())
	{
		for (auto &subslot : m_cartslot)
		{
			auto target = subslot.finder_target();
			parent_slot()->configure_subslot(*target.first.subdevice<msx_slot_cartridge_device>(target.second));
		}
	}
}

void msx_cart_slotexpander_device::device_resolve_objects()
{
	for (int pg = 0; pg < 4; pg++)
		page(pg)->install_view(0x4000 * pg, 0x04000 * pg + 0x3fff, m_view[pg]);
	page(3)->install_readwrite_handler(0xffff, 0xffff, emu::rw_delegate(*this, FUNC(msx_cart_slotexpander_device::secondary_slot_r)), emu::rw_delegate(*this, FUNC(msx_cart_slotexpander_device::secondary_slot_w)));

	for (int subslot = 0; subslot < 4; subslot++)
		for (int page = 0; page < 4; page++)
			m_view[page][subslot];

	for (int subslot = 0; subslot < 4; subslot++)
		m_cartslot[subslot]->install(&m_view[0][subslot], &m_view[1][subslot], &m_view[2][subslot], &m_view[3][subslot]);
}

void msx_cart_slotexpander_device::device_start()
{
	save_item(NAME(m_secondary_slot));
	m_secondary_slot = 0;

	for (int page = 0; page < 4; page++)
		m_view[page].select(0);
}

u8 msx_cart_slotexpander_device::secondary_slot_r()
{
	return ~m_secondary_slot;
}

void msx_cart_slotexpander_device::secondary_slot_w(u8 data)
{
	for (int page = 0; page < 4; page++)
		m_view[page].select((data >> (2 * page)) & 0x03);
	m_secondary_slot = data;
}


class msx_cart_wondertang_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_wondertang_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_WONDERTANG, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_subslot(*this, "subslot%u", 0U)
		, m_franky(*this, "franky")
		, m_irq_out(*this, "irq_out")
		, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"} }
		, m_secondary_slot(0)
	{ }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	template <unsigned Slot> void add_subslot(machine_config &config, char const *option, device_type type);
	u8 secondary_slot_r();
	void secondary_slot_w(u8 data);

	required_device_array<msx_slot_cartridge_device, 4> m_subslot;
	required_device<msx_slot_cartridge_device> m_franky;
	required_device<input_merger_any_high_device> m_irq_out;
	memory_view m_view[4];
	u8 m_secondary_slot;
};

ROM_START(wondertang)
	ROM_REGION(0x20000, "dos", 0)
	ROM_LOAD("Nextor-3.0.0-beta1.WonderTANG.ROM", 0, 0x20000, CRC(5f02462f) SHA1(3d390267d6e0cb53431870c3730c9f9a8150dcf7))

	ROM_REGION(0x4000, "fmpac", 0)
	ROM_LOAD("16k_fm_opl.bin", 0, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))

	ROM_REGION(0x4000, "sfg", 0)
	ROM_LOAD("sfg01.rom", 0, 0x4000, CRC(84f4b692) SHA1(49a1750c10e407293af6bce27a02e99307ceba12))
ROM_END

const tiny_rom_entry *msx_cart_wondertang_device::device_rom_region() const
{
	return ROM_NAME(wondertang);
}

template <unsigned Slot>
void msx_cart_wondertang_device::add_subslot(machine_config &config, char const *option, device_type type)
{
	MSX_SLOT_CARTRIDGE(config, m_subslot[Slot], DERIVED_CLOCK(1, 1));
	m_subslot[Slot]->option_reset();
	msx_cart(*m_subslot[Slot], true);
	m_subslot[Slot]->option_add_internal(option, type);
	m_subslot[Slot]->set_default_option(option);
	m_subslot[Slot]->set_fixed(true);
	m_subslot[Slot]->irq_handler().set(m_irq_out, FUNC(input_merger_device::in_w<Slot>));
	if (parent_slot())
		m_subslot[Slot]->add_route(ALL_OUTPUTS, soundin(), 1.0);
}

void msx_cart_wondertang_device::device_add_mconfig(machine_config &config)
{
	add_subslot<0>(config, "wondertang_sdd", MSX_CART_WONDERTANG_SDD);
	add_subslot<1>(config, "wondertang_fmpac", MSX_CART_WONDERTANG_FMPAC);
	add_subslot<2>(config, "wondertang_megaram", MSX_CART_WONDERTANG_MEGARAM);
	add_subslot<3>(config, "wondertang_mm", MSX_CART_WONDERTANG_MM);

	MSX_SLOT_CARTRIDGE(config, m_franky, DERIVED_CLOCK(1, 1));
	m_franky->option_reset();
	msx_cart(*m_franky, true);
	m_franky->set_default_option(bus::msx::cart::slotoptions::FRANKY);
	m_franky->set_fixed(true);
	m_franky->irq_handler().set(m_irq_out, FUNC(input_merger_device::in_w<4>));
	if (parent_slot())
		m_franky->add_route(ALL_OUTPUTS, soundin(), 1.0);

	INPUT_MERGER_ANY_HIGH(config, m_irq_out).output_handler().set(*this, FUNC(msx_cart_wondertang_device::irq_out));
}

void msx_cart_wondertang_device::device_config_complete()
{
	if (parent_slot())
	{
		for (auto &subslot : m_subslot)
		{
			auto const target = subslot.finder_target();
			parent_slot()->configure_subslot(*target.first.subdevice<msx_slot_cartridge_device>(target.second));
		}
		auto const target = m_franky.finder_target();
		parent_slot()->configure_subslot(*target.first.subdevice<msx_slot_cartridge_device>(target.second));
	}
}

void msx_cart_wondertang_device::device_resolve_objects()
{
	for (unsigned page = 0; page < 4; ++page)
		this->page(page)->install_view(0x4000 * page, 0x4000 * page + 0x3fff, m_view[page]);
	this->page(3)->install_readwrite_handler(0xffff, 0xffff,
		emu::rw_delegate(*this, FUNC(msx_cart_wondertang_device::secondary_slot_r)),
		emu::rw_delegate(*this, FUNC(msx_cart_wondertang_device::secondary_slot_w)));

	for (unsigned subslot = 0; subslot < 4; ++subslot)
	{
		for (unsigned page = 0; page < 4; ++page)
			m_view[page][subslot];
		m_subslot[subslot]->install(&m_view[0][subslot], &m_view[1][subslot], &m_view[2][subslot], &m_view[3][subslot]);
	}

	// Franky is selected entirely through I/O ports, so it shares the bus but
	// has no secondary-slot memory destination.
	m_franky->install(&m_view[0][0], &m_view[1][0], &m_view[2][0], &m_view[3][0]);
}

void msx_cart_wondertang_device::device_start()
{
	save_item(NAME(m_secondary_slot));
	m_secondary_slot = 0;
	for (auto &view : m_view)
		view.select(0);
}

u8 msx_cart_wondertang_device::secondary_slot_r()
{
	return ~m_secondary_slot;
}

void msx_cart_wondertang_device::secondary_slot_w(u8 data)
{
	for (unsigned page = 0; page < 4; ++page)
		m_view[page].select(BIT(data, 2 * page, 2));
	m_secondary_slot = data;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SLOTEXPANDER, msx_cart_interface, msx_cart_slotexpander_device, "msx_cart_slotexpander", "MSX Slot Expander")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_WONDERTANG, msx_cart_interface, msx_cart_wondertang_device, "msx_cart_wondertang", "WonderTANG New Juice")
