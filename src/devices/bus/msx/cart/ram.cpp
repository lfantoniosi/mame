// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
Emulation of memory expansions for the MSX system.

Standard 16K RAM expansions:
- Addcom ADMSM 301
- Casio OR-216
- General RJ-102
- Hitachi MP-RA01H
- Mitsubishi ML-16RAM
- National CF-2131
- Philips VU 0031
- Sanyo MRP-16
- Sony HBM-16
- Toshiba HX-M250

Standard 32K RAM expansions:
- Addcom ADMSM 302
- Pioneer PX-RA32
- Yamaha URM-01

Standard 48K RAM expansions:
- Addcom ADMSM 303
- Neos RM-48

Standard 64K RAM expansions:
- Casio OR-264
- Philips VU 0034
- Sanyo Memoire Ram 64Ko
- Sanyo MRP-64
- Sharp-Epcom HB-4100
- Sony HBM-64
- Spectravideo SVI-747
- Toshiba HX-M251
- Walther Miller WM 0587

Not supported:
- Fujitsu MB22451 - Fujitsu FM-X only, adds 16K memory and a printer port
- Yamaha SRE-01 - 60pin 32K memory expansion
- Yamaha SRM-01 - 60pin 32K memory expansion and slot expander



256KB Memmory Mapper expansions
- R. & J.Jansen Memory Mapper 256

512KB Memory Mapper expansions
- Green MSX WBM512
- Hardware Partners TM 512KB
- HSH RE-512 MM
- Padial LPE-512KBSRAM-V3
- R. & J.Jansen Memory Mapper 512
- Sony HBM-512

768KB Memory Mapper expansions
- ASCII MEM-768

1MB Memory Mapper expansions
- 8bits4ever MEGA-MAPPER 1024KB
- Digital KC MSX Memory Mapper 1024KB
- Hardware Partners TM 1M
- MK MSX Memory Mapper 1024KB
- MSX Club Gouda Memory Mapper 1MB
- Popolon MSX Memory Mapper

2MB Memory Mapper expansions
- 8bits4ever MEGA-MAPPER 2048KB
- Digital KC MSX Memory Mapper 2048KB
- MFP Extended Memory-2048K
- MK MSX Memory Mapper 2048KB
- MSX Club Gouda Memory Mapper 2MB

4MB Memory Mapper expansions
- 8bits4ever MEGA-MAPPER 4096KB
- Digital KC MSX Memory Mapper 4096KB
- MK MSX Memory Mapper 4096KB
- MSX Club Gouda Memory Mapper 4MB
- Padial LPE-4MB-V42KP
- T.N.S. Addram
- Tecnobytes Double RAM

Known issues/problems:
- Switching Tecnobytes Double RAM from Memory Mapper to  MegaRAM mode will cause
  the emulation to look up when the system is using the Double RAM as main
  memory. It is unknown if this also happens on a real system.

TODO:
- Unknown which parts of memory are used for the MegaRAM 8KB pages.


Not supported memory mappers:
- 8bits4ever SD-512
- RBSC Carnivore2
- Classic PC 16MB Expansion Memory - 4 x 4MB memory mapper
- GR8NET
- MSX Cartridge Shop MegaFlashROM SCC+ SD
- Padial LPE-4FMB-V8SKP - 4MB memory mapper + MegaRam mode
- Playsonic - 4MB memory mapper, up to 16MB through extra register
- Stichting CODE MCR-025 - 256KB memory mapper + RAM Disk + Printer buffer
- Stichting CODE MCR-051 - 512KB memory mapper + RAM Disk + Printer buffer
- Stichting CODE MCR-076 - 768KB memory mapper + RAM Disk + Printer buffer
- Stichting CODE MCR-102 - 1MB memory mapper + RAM Disk + Printer buffer
- Stichting CODE MCR-204 - 2MB memory mapper + RAM Disk + Printer buffer
*/
#include "emu.h"
#include "ram.h"
#include "slotoptions.h"

#include "bus/msx/slot/cartridge.h"
#include "bus/generic/slot.h"
#include "sound/k051649.h"
#include "sound/sn76496.h"



DECLARE_DEVICE_TYPE(MSX_CART_16K_RAM,      msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_32K_RAM,      msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_48K_RAM,      msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_64K_RAM,      msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_256K_MM_RAM,  msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_512K_MM_RAM,  msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_768K_MM_RAM,  msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_1024K_MM_RAM, msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_2048K_MM_RAM, msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_4096K_MM_RAM, msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_DOUBLE_RAM,   msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_MMM,          msx_cart_interface)

void msx_cart_ram_register_options(device_slot_interface &device)
{
	using namespace bus::msx::cart;
	device.option_add(slotoptions::RAM16K,     MSX_CART_16K_RAM);
	device.option_add(slotoptions::RAM32K,     MSX_CART_32K_RAM);
	device.option_add(slotoptions::RAM48K,     MSX_CART_48K_RAM);
	device.option_add(slotoptions::RAM64K,     MSX_CART_64K_RAM);
	device.option_add(slotoptions::MM256K,     MSX_CART_256K_MM_RAM);
	device.option_add(slotoptions::MM512K,     MSX_CART_512K_MM_RAM);
	device.option_add(slotoptions::MM768K,     MSX_CART_768K_MM_RAM);
	device.option_add(slotoptions::MM1024K,    MSX_CART_1024K_MM_RAM);
	device.option_add(slotoptions::MM2048K,    MSX_CART_2048K_MM_RAM);
	device.option_add(slotoptions::MM4096K,    MSX_CART_4096K_MM_RAM);
	device.option_add(slotoptions::DOUBLE_RAM, MSX_CART_DOUBLE_RAM);
	device.option_add(slotoptions::MMM,        MSX_CART_MMM);
}

namespace {

class msx_cart_base_ram_device : public device_t, public msx_cart_interface
{
protected:
	msx_cart_base_ram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 ram_size)
		: device_t(mconfig, type, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_ram_size(ram_size)
	{ }

	virtual void device_start() override
	{
		m_ram = std::make_unique<u8[]>(m_ram_size);
		save_pointer(NAME(m_ram), m_ram_size);
	}

	u8 *ram_data()
	{
		return m_ram.get();
	}

	u32 get_ram_size() const
	{
		return m_ram_size;
	}

private:
	std::unique_ptr<u8[]> m_ram;
	const u32 m_ram_size;
};


class msx_cart_16k_ram_device : public msx_cart_base_ram_device
{
public:
	msx_cart_16k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_ram_device(mconfig, MSX_CART_16K_RAM, tag, owner, clock, 16 * 1024)
	{ }

protected:
	virtual void device_start() override
	{
		msx_cart_base_ram_device::device_start();
		page(2)->install_ram(0x8000, 0xbfff, ram_data());
	}
};

class msx_cart_32k_ram_device : public msx_cart_base_ram_device
{
public:
	msx_cart_32k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_ram_device(mconfig, MSX_CART_32K_RAM, tag, owner, clock, 32 * 1024)
	{ }

protected:
	virtual void device_start() override
	{
		msx_cart_base_ram_device::device_start();
		page(0)->install_ram(0x0000, 0x3fff, ram_data());
		page(1)->install_ram(0x4000, 0x7fff, ram_data() + 0x4000);
	}
};

class msx_cart_48k_ram_device : public msx_cart_base_ram_device
{
public:
	msx_cart_48k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_ram_device(mconfig, MSX_CART_48K_RAM, tag, owner, clock, 48 * 1024)
	{ }

protected:
	virtual void device_start() override
	{
		msx_cart_base_ram_device::device_start();
		page(0)->install_ram(0x0000, 0x3fff, ram_data());
		page(1)->install_ram(0x4000, 0x7fff, ram_data() + 0x4000);
		page(2)->install_ram(0x8000, 0xbfff, ram_data() + 0x8000);
	}
};

class msx_cart_64k_ram_device : public msx_cart_base_ram_device
{
public:
	msx_cart_64k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_ram_device(mconfig, MSX_CART_64K_RAM, tag, owner, clock, 64 * 1024)
	{ }

protected:
	virtual void device_start() override
	{
		msx_cart_base_ram_device::device_start();
		page(0)->install_ram(0x0000, 0x3fff, ram_data());
		page(1)->install_ram(0x4000, 0x7fff, ram_data() + 0x4000);
		page(2)->install_ram(0x8000, 0xbfff, ram_data() + 0x8000);
		page(3)->install_ram(0xc000, 0xffff, ram_data() + 0xc000);
	}
};


class msx_cart_base_mm_ram_device : public msx_cart_base_ram_device
{
protected:
	msx_cart_base_mm_ram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 ram_size)
		: msx_cart_base_ram_device(mconfig, type, tag, owner, clock, ram_size)
		, m_rambank(*this, "rambank%u", 0U)
		, m_bank_mask(0)
	{ }

	virtual void device_start() override ATTR_COLD;

	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<4> m_rambank;
	u8 m_bank_mask;
};

void msx_cart_base_mm_ram_device::device_start()
{
	msx_cart_base_ram_device::device_start();

	m_bank_mask = device_generic_cart_interface::map_non_power_of_two(
			unsigned(get_ram_size() / 0x4000),
			[this] (unsigned entry, unsigned page)
			{
				for (int i = 0; i < 4; i++)
					m_rambank[i]->configure_entry(entry, ram_data() + 0x4000 * page);
			}
	);

	// The MSX system allows multiple devices to react to I/O, so we use taps.
	io_space().install_write_tap(0xfc, 0xfc, "bank0", [this] (offs_t, u8& data, u8){ this->bank_w<0>(data); });
	io_space().install_write_tap(0xfd, 0xfd, "bank1", [this] (offs_t, u8& data, u8){ this->bank_w<1>(data); });
	io_space().install_write_tap(0xfe, 0xfe, "bank2", [this] (offs_t, u8& data, u8){ this->bank_w<2>(data); });
	io_space().install_write_tap(0xff, 0xff, "bank3", [this] (offs_t, u8& data, u8){ this->bank_w<3>(data); });

	page(0)->install_readwrite_bank(0x0000, 0x3fff, m_rambank[0]);
	page(1)->install_readwrite_bank(0x4000, 0x7fff, m_rambank[1]);
	page(2)->install_readwrite_bank(0x8000, 0xbfff, m_rambank[2]);
	page(3)->install_readwrite_bank(0xc000, 0xffff, m_rambank[3]);

	m_rambank[0]->set_entry(3);
	m_rambank[1]->set_entry(2);
	m_rambank[2]->set_entry(1);
	m_rambank[3]->set_entry(0);
}

template <int Bank>
void msx_cart_base_mm_ram_device::bank_w(u8 data)
{
	m_rambank[Bank]->set_entry(data & m_bank_mask);
}


class msx_cart_256k_mm_ram_device : public msx_cart_base_mm_ram_device
{
public:
	msx_cart_256k_mm_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_mm_ram_device(mconfig, MSX_CART_256K_MM_RAM, tag, owner, clock, 256 * 1024)
	{ }
};

class msx_cart_512k_mm_ram_device : public msx_cart_base_mm_ram_device
{
public:
	msx_cart_512k_mm_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_mm_ram_device(mconfig, MSX_CART_512K_MM_RAM, tag, owner, clock, 512 * 1024)
	{ }
};

class msx_cart_768k_mm_ram_device : public msx_cart_base_mm_ram_device
{
public:
	msx_cart_768k_mm_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_mm_ram_device(mconfig, MSX_CART_768K_MM_RAM, tag, owner, clock, 768 * 1024)
	{ }
};

class msx_cart_1024k_mm_ram_device : public msx_cart_base_mm_ram_device
{
public:
	msx_cart_1024k_mm_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_mm_ram_device(mconfig, MSX_CART_1024K_MM_RAM, tag, owner, clock, 1024 * 1024)
	{ }
};

class msx_cart_2048k_mm_ram_device : public msx_cart_base_mm_ram_device
{
public:
	msx_cart_2048k_mm_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_mm_ram_device(mconfig, MSX_CART_2048K_MM_RAM, tag, owner, clock, 2048 * 1024)
	{ }
};

class msx_cart_4096k_mm_ram_device : public msx_cart_base_mm_ram_device
{
public:
	msx_cart_4096k_mm_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_mm_ram_device(mconfig, MSX_CART_4096K_MM_RAM, tag, owner, clock, 4096 * 1024)
	{ }
};


class msx_cart_mmm_device : public msx_cart_base_mm_ram_device
{
public:
	msx_cart_mmm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_mm_ram_device(mconfig, MSX_CART_MMM, tag, owner, clock, 1024 * 1024)
		, m_sn76489a(*this, "sn76489a")
		, m_access_enabled(false)
		, m_sn76489_enabled(false)
	{ }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<sn76496_base_device> m_sn76489a;
	bool m_access_enabled;
	bool m_sn76489_enabled;
};

void msx_cart_mmm_device::device_start()
{
	msx_cart_base_mm_ram_device::device_start();

	m_access_enabled = false;
	m_sn76489_enabled = false;

	io_space().install_write_tap(0x3c, 0x3c, "ena_access", [this] (offs_t, u8& data, u8){ this->m_access_enabled = BIT(data, 7); });
	io_space().install_write_tap(0x3f, 0x3f, "sn76489w", [this] (offs_t, u8& data, u8){ if (m_sn76489_enabled) this->m_sn76489a->write(data); });

	page(2)->install_write_tap(0x803c, 0x803c, "ena_sn76489", [this] (offs_t, u8& data, u8){ if (m_access_enabled) this->m_sn76489_enabled |= BIT(data, 6); });
	page(2)->install_read_tap(0x80fc, 0x80fc, "map00r", [this] (offs_t, u8& data, u8){ if (m_access_enabled) data = 0xc0 | this->m_rambank[0]->entry(); });
	page(2)->install_read_tap(0x80fd, 0x80fd, "map40r", [this] (offs_t, u8& data, u8){ if (m_access_enabled) data = 0xc0 | this->m_rambank[1]->entry(); });
	page(2)->install_read_tap(0x80fe, 0x80fe, "map80r", [this] (offs_t, u8& data, u8){ if (m_access_enabled) data = 0xc0 | this->m_rambank[2]->entry(); });
	page(2)->install_read_tap(0x80ff, 0x80ff, "mapc0r", [this] (offs_t, u8& data, u8){ if (m_access_enabled) data = 0xc0 | this->m_rambank[3]->entry(); });
	page(2)->install_write_tap(0x80fc, 0x80fc, "map00w", [this] (offs_t, u8& data, u8){ if (m_access_enabled) this->bank_w<0>(data); });
	page(2)->install_write_tap(0x80fd, 0x80fd, "map40w", [this] (offs_t, u8& data, u8){ if (m_access_enabled) this->bank_w<1>(data); });
	page(2)->install_write_tap(0x80fe, 0x80fe, "map80w", [this] (offs_t, u8& data, u8){ if (m_access_enabled) this->bank_w<2>(data); });
	page(2)->install_write_tap(0x80ff, 0x80ff, "mapc0w", [this] (offs_t, u8& data, u8){ if (m_access_enabled) this->bank_w<3>(data); });

	save_item(NAME(m_access_enabled));
	save_item(NAME(m_sn76489_enabled));
}

void msx_cart_mmm_device::device_add_mconfig(machine_config &config)
{
	SN76489A(config, m_sn76489a, DERIVED_CLOCK(1, 3));
	if (parent_slot())
		m_sn76489a->add_route(ALL_OUTPUTS, soundin(), 0.8);
}



class msx_cart_double_ram_device : public msx_cart_base_mm_ram_device
{
public:
	msx_cart_double_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_base_mm_ram_device(mconfig, MSX_CART_DOUBLE_RAM, tag, owner, clock, 4 * 1024 * 1024)
		, m_mode_switch(*this, "MODE")
		, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"} }
		, m_mrview{ {*this, "mrview0"}, {*this, "mrview1"}, {*this, "mrview2"}, {*this, "mrview3"} }
		, m_megaram(false)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(mode_callback);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	static constexpr u8 MODE_RO = 0;
	static constexpr u8 MODE_RW = 1;
	static constexpr u8 MODE_MM = 0;
	static constexpr u8 MODE_MR = 1;

	required_ioport m_mode_switch;
	memory_view m_view[4];
	memory_view m_mrview[4];
	bool m_megaram;

	void set_page_views();
};

static INPUT_PORTS_START(double_ram_mode_switch)
	PORT_START("MODE")
	PORT_CONFNAME(0x01, 0x01, "Mode") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(msx_cart_double_ram_device::mode_callback), 0)
	PORT_CONFSETTING(0x00, "Memory Mapper")
	PORT_CONFSETTING(0x01, "MegaRAM")
INPUT_PORTS_END

ioport_constructor msx_cart_double_ram_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(double_ram_mode_switch);
}

void msx_cart_double_ram_device::device_start()
{
	msx_cart_base_ram_device::device_start();

	m_bank_mask = device_generic_cart_interface::map_non_power_of_two(
			unsigned(get_ram_size() / 0x4000),
			[this] (unsigned entry, unsigned page)
			{
				for (int i = 0; i < 4; i++)
					m_rambank[i]->configure_entry(entry, ram_data() + 0x4000 * page);
			}
	);

	page(0)->install_view(0x0000, 0x3fff, m_view[0]);
	page(1)->install_view(0x4000, 0x7fff, m_view[1]);
	page(2)->install_view(0x8000, 0xbfff, m_view[2]);
	page(3)->install_view(0xc000, 0xffff, m_view[3]);

	m_view[0][MODE_MM].install_readwrite_bank(0x0000, 0x3fff, m_rambank[0]);
	m_view[0][MODE_MR];

	m_view[1][MODE_MM].install_readwrite_bank(0x4000, 0x7fff, m_rambank[1]);
	m_view[1][MODE_MR].install_view(0x4000, 0x5fff, m_mrview[0]);
	m_view[1][MODE_MR].install_view(0x6000, 0x7fff, m_mrview[1]);

	m_view[2][MODE_MM].install_readwrite_bank(0x8000, 0xbfff, m_rambank[2]);
	m_view[2][MODE_MR].install_view(0x8000, 0x9fff, m_mrview[2]);
	m_view[2][MODE_MR].install_view(0xa000, 0xbfff, m_mrview[3]);

	m_view[3][MODE_MM].install_readwrite_bank(0xc000, 0xffff, m_rambank[3]);
	m_view[3][MODE_MR];

	m_mrview[0][MODE_RO].install_read_bank(0x4000, 0x5fff, m_rambank[0]);
	m_mrview[0][MODE_RO].install_write_handler(0x4000, 0x4000, write8smo_delegate(*this, [this] (u8 data) { this->m_rambank[0]->set_entry(data); }, "bank_4000"));
	m_mrview[0][MODE_RW].install_readwrite_bank(0x4000, 0x5fff, m_rambank[0]);

	m_mrview[1][MODE_RO].install_read_bank(0x6000, 0x7fff, m_rambank[1]);
	m_mrview[1][MODE_RO].install_write_handler(0x6000, 0x6000, write8smo_delegate(*this, [this] (u8 data) { this->m_rambank[1]->set_entry(data); }, "bank_6000"));
	m_mrview[1][MODE_RW].install_readwrite_bank(0x6000, 0x7fff, m_rambank[1]);

	m_mrview[2][MODE_RO].install_read_bank(0x8000, 0x9fff, m_rambank[2]);
	m_mrview[2][MODE_RO].install_write_handler(0x8000, 0x8000, write8smo_delegate(*this, [this] (u8 data) { this->m_rambank[2]->set_entry(data); }, "bank_8000"));
	m_mrview[2][MODE_RW].install_readwrite_bank(0x8000, 0x9fff, m_rambank[2]);

	m_mrview[3][MODE_RO].install_read_bank(0xa000, 0xbfff, m_rambank[3]);
	m_mrview[3][MODE_RO].install_write_handler(0xa000, 0xa000, write8smo_delegate(*this, [this] (u8 data) { this->m_rambank[3]->set_entry(data); }, "bank_a000"));
	m_mrview[3][MODE_RW].install_readwrite_bank(0xa000, 0xbfff, m_rambank[3]);

	io_space().install_read_tap(0x8e, 0x8e, "megaram_w", [this] (offs_t, u8&, u8) { if (m_megaram) for (int i = 0; i < 4; i++) m_mrview[i].select(MODE_RW); } );
	io_space().install_write_tap(0x8e, 0x8e, "megaram_r", [this] (offs_t, u8&, u8) { if (m_megaram) for (int i = 0; i < 4; i++) m_mrview[i].select(MODE_RO); } );

	// The MSX system allows multiple devices to react to I/O, so we use taps.
	io_space().install_write_tap(0xfc, 0xfc, "bank0", [this] (offs_t, u8& data, u8){ if (!m_megaram) this->bank_w<0>(data); });
	io_space().install_write_tap(0xfd, 0xfd, "bank1", [this] (offs_t, u8& data, u8){ if (!m_megaram) this->bank_w<1>(data); });
	io_space().install_write_tap(0xfe, 0xfe, "bank2", [this] (offs_t, u8& data, u8){ if (!m_megaram) this->bank_w<2>(data); });
	io_space().install_write_tap(0xff, 0xff, "bank3", [this] (offs_t, u8& data, u8){ if (!m_megaram) this->bank_w<3>(data); });

	save_item(NAME(m_megaram));

	for (int i = 0; i < 4; i++)
	{
		m_mrview[i].select(MODE_RO);
		m_rambank[i]->set_entry(i);
	}
}

void msx_cart_double_ram_device::device_reset()
{
	m_megaram = BIT(m_mode_switch->read(), 0);
	set_page_views();
}

INPUT_CHANGED_MEMBER(msx_cart_double_ram_device::mode_callback)
{
	m_megaram = BIT(newval, 0);
	set_page_views();
}

void msx_cart_double_ram_device::set_page_views()
{
	for (int i = 0; i < 4; i++)
		m_view[i].select(m_megaram ? MODE_MR : MODE_MM);
}


class msx_cart_wondertang_megaram_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_wondertang_megaram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_WONDERTANG_MEGARAM, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_k051649(*this, "k051649")
	{ }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr u8 MODE_DDX_SCC = 0x00;
	static constexpr u8 MODE_DDX = 0x01;
	static constexpr u8 MODE_LINEAR = 0x02;
	static constexpr u8 MODE_K4 = 0x04;
	static constexpr u8 MODE_K5 = 0x05;
	static constexpr u8 MODE_ASCII8 = 0x08;
	static constexpr u8 MODE_ASCII16 = 0x16;

	template <unsigned Page> u8 memory_r(offs_t offset);
	template <unsigned Page> void memory_w(offs_t offset, u8 data);
	void mapper_w(u8 data);
	bool bank_register_w(u16 address, u8 data);
	u32 ram_address(u16 address) const;
	bool scc_active(u16 address) const;
	u8 scc_r(u16 address);
	void scc_w(u16 address, u8 data);

	required_device<k051649_device> m_k051649;
	std::unique_ptr<u8[]> m_ram;
	u8 m_bank[4]{};
	u8 m_mode = MODE_DDX_SCC;
	bool m_rom_mode = true;
};

void msx_cart_wondertang_megaram_device::device_add_mconfig(machine_config &config)
{
	// K051649 is the SCC sound/register block only.  Super-MegaRAM remains
	// solely responsible for all mapper registers and SDRAM bank selection.
	K051649(config, m_k051649, DERIVED_CLOCK(1, 1));
	if (parent_slot())
		m_k051649->add_route(ALL_OUTPUTS, soundin(), 0.8);
}

void msx_cart_wondertang_megaram_device::device_start()
{
	m_ram = std::make_unique<u8[]>(2 * 1024 * 1024);
	save_pointer(NAME(m_ram), 2 * 1024 * 1024);
	save_item(NAME(m_bank));
	save_item(NAME(m_mode));
	save_item(NAME(m_rom_mode));

	page(0)->install_readwrite_handler(0x0000, 0x3fff,
		read8sm_delegate(*this, FUNC(msx_cart_wondertang_megaram_device::memory_r<0>)),
		write8sm_delegate(*this, FUNC(msx_cart_wondertang_megaram_device::memory_w<0>)));
	page(1)->install_readwrite_handler(0x4000, 0x7fff,
		read8sm_delegate(*this, FUNC(msx_cart_wondertang_megaram_device::memory_r<1>)),
		write8sm_delegate(*this, FUNC(msx_cart_wondertang_megaram_device::memory_w<1>)));
	page(2)->install_readwrite_handler(0x8000, 0xbfff,
		read8sm_delegate(*this, FUNC(msx_cart_wondertang_megaram_device::memory_r<2>)),
		write8sm_delegate(*this, FUNC(msx_cart_wondertang_megaram_device::memory_w<2>)));
	page(3)->install_readwrite_handler(0xc000, 0xffff,
		read8sm_delegate(*this, FUNC(msx_cart_wondertang_megaram_device::memory_r<3>)),
		write8sm_delegate(*this, FUNC(msx_cart_wondertang_megaram_device::memory_w<3>)));

	// Like the FPGA, neither control port drives the data bus on reads.
	io_space().install_read_tap(0x8e, 0x8e, "wondertang_megaram_write", [this] (offs_t, u8 &, u8) { m_rom_mode = false; });
	io_space().install_write_tap(0x8e, 0x8e, "wondertang_megaram_rom", [this] (offs_t, u8 &, u8) { m_rom_mode = true; });
	io_space().install_write_tap(0x8f, 0x8f, "wondertang_megaram_mapper", [this] (offs_t, u8 &data, u8) { mapper_w(data); });
}

void msx_cart_wondertang_megaram_device::device_reset()
{
	for (unsigned bank = 0; bank < 4; ++bank)
		m_bank[bank] = bank;
	m_mode = MODE_DDX_SCC;
	m_rom_mode = true;
}

void msx_cart_wondertang_megaram_device::mapper_w(u8 data)
{
	// LINEAR is latched until reset in the current New Juice RTL.
	if (m_mode == MODE_LINEAR)
		return;

	switch (data)
	{
	case MODE_DDX_SCC:
	case MODE_DDX:
	case MODE_LINEAR:
	case MODE_K4:
	case MODE_K5:
	case MODE_ASCII8:
	case MODE_ASCII16:
		m_mode = data;
		break;
	default:
		m_mode = MODE_DDX_SCC;
		break;
	}
}

bool msx_cart_wondertang_megaram_device::bank_register_w(u16 address, u8 data)
{
	unsigned bank = 0;
	bool selected = false;

	switch (m_mode)
	{
	case MODE_DDX_SCC:
	case MODE_DDX:
		if (!(address & 0x0fff) && address >= 0x4000 && address < 0xc000)
		{
			bank = (address - 0x4000) >> 13;
			selected = true;
		}
		break;

	case MODE_ASCII8:
		if (address >= 0x6000 && address <= 0x7fff)
		{
			bank = (address - 0x6000) >> 11;
			selected = true;
		}
		break;

	case MODE_ASCII16:
		if (address >= 0x6000 && address <= 0x67ff)
		{
			bank = 0;
			selected = true;
		}
		else if (address >= 0x7000 && address <= 0x77ff)
		{
			bank = 2;
			selected = true;
		}
		break;

	case MODE_K4:
		if (address >= 0x4000 && address < 0xc000)
		{
			bank = (address - 0x4000) >> 13;
			selected = true;
		}
		break;

	case MODE_K5:
		if ((address >= 0x5000 && address <= 0x57ff) ||
			(address >= 0x7000 && address <= 0x77ff) ||
			(address >= 0x9000 && address <= 0x97ff) ||
			(address >= 0xb000 && address <= 0xb7ff))
		{
			bank = (address - 0x5000) >> 13;
			selected = true;
		}
		break;
	}

	if (selected)
		m_bank[bank] = data;
	return selected;
}

u32 msx_cart_wondertang_megaram_device::ram_address(u16 address) const
{
	if (m_mode == MODE_LINEAR)
		return address;
	if (m_mode == MODE_ASCII16)
	{
		u8 const bank = address < 0x8000 ? m_bank[0] : m_bank[2];
		return (u32(bank & 0x7f) << 14) | (address & 0x3fff);
	}

	unsigned const segment = (address - 0x4000) >> 13;
	return (u32(m_bank[segment]) << 13) | (address & 0x1fff);
}

bool msx_cart_wondertang_megaram_device::scc_active(u16 address) const
{
	return m_rom_mode &&
		(m_mode == MODE_DDX_SCC || m_mode == MODE_K5) &&
		(m_bank[2] == 0x3f) &&
		(address >= 0x9800 && address <= 0x9fff);
}

u8 msx_cart_wondertang_megaram_device::scc_r(u16 address)
{
	u8 const reg = u8(address);
	if (reg <= 0x7f)
		return m_k051649->k051649_waveform_r(reg);
	if (reg >= 0xe0)
		return m_k051649->k051649_test_r(memory_space());
	return 0xff;
}

void msx_cart_wondertang_megaram_device::scc_w(u16 address, u8 data)
{
	u8 const reg = u8(address);
	if (reg <= 0x7f)
		m_k051649->k051649_waveform_w(reg, data);
	else if ((reg >= 0x80 && reg <= 0x89) || (reg >= 0x90 && reg <= 0x99))
		m_k051649->k051649_frequency_w(reg & 0x0f, data);
	else if ((reg >= 0x8a && reg <= 0x8e) || (reg >= 0x9a && reg <= 0x9e))
		m_k051649->k051649_volume_w((reg & 0x0f) - 0x0a, data);
	else if (reg == 0x8f || reg == 0x9f)
		m_k051649->k051649_keyonoff_w(data);
	else if (reg >= 0xe0)
		m_k051649->k051649_test_w(data);
}

template <unsigned Page>
u8 msx_cart_wondertang_megaram_device::memory_r(offs_t offset)
{
	u16 const address = (Page << 14) | offset;
	if (m_mode != MODE_LINEAR && (address < 0x4000 || address >= 0xc000))
		return 0xff;
	if (scc_active(address))
		return scc_r(address);
	return m_ram[ram_address(address)];
}

template <unsigned Page>
void msx_cart_wondertang_megaram_device::memory_w(offs_t offset, u8 data)
{
	u16 const address = (Page << 14) | offset;
	if (m_mode != MODE_LINEAR && (address < 0x4000 || address >= 0xc000))
		return;
	if (scc_active(address))
	{
		scc_w(address, data);
		return;
	}
	if (!m_rom_mode)
		m_ram[ram_address(address)] = data;
	else
		bank_register_w(address, data);
}


class msx_cart_wondertang_mm_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_wondertang_mm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_WONDERTANG_MM, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_bank(*this, "bank%u", 0U)
	{ }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <unsigned Bank> void bank_w(offs_t, u8 &data, u8) { m_bank[Bank]->set_entry(data); }
	template <unsigned Bank> void bank_r(offs_t, u8 &data, u8) { data = m_bank[Bank]->entry(); }

	std::unique_ptr<u8[]> m_ram;
	memory_bank_array_creator<4> m_bank;
};

void msx_cart_wondertang_mm_device::device_start()
{
	m_ram = std::make_unique<u8[]>(4 * 1024 * 1024);
	save_pointer(NAME(m_ram), 4 * 1024 * 1024);

	for (unsigned page_no = 0; page_no < 4; ++page_no)
	{
		m_bank[page_no]->configure_entries(0, 256, m_ram.get(), 0x4000);
		page(page_no)->install_readwrite_bank(page_no * 0x4000, page_no * 0x4000 + 0x3fff, m_bank[page_no]);
	}

	io_space().install_read_tap(0xfc, 0xfc, "wondertang_mm_r0", [this] (offs_t o, u8 &d, u8 m) { bank_r<0>(o, d, m); });
	io_space().install_read_tap(0xfd, 0xfd, "wondertang_mm_r1", [this] (offs_t o, u8 &d, u8 m) { bank_r<1>(o, d, m); });
	io_space().install_read_tap(0xfe, 0xfe, "wondertang_mm_r2", [this] (offs_t o, u8 &d, u8 m) { bank_r<2>(o, d, m); });
	io_space().install_read_tap(0xff, 0xff, "wondertang_mm_r3", [this] (offs_t o, u8 &d, u8 m) { bank_r<3>(o, d, m); });
	io_space().install_write_tap(0xfc, 0xfc, "wondertang_mm_w0", [this] (offs_t o, u8 &d, u8 m) { bank_w<0>(o, d, m); });
	io_space().install_write_tap(0xfd, 0xfd, "wondertang_mm_w1", [this] (offs_t o, u8 &d, u8 m) { bank_w<1>(o, d, m); });
	io_space().install_write_tap(0xfe, 0xfe, "wondertang_mm_w2", [this] (offs_t o, u8 &d, u8 m) { bank_w<2>(o, d, m); });
	io_space().install_write_tap(0xff, 0xff, "wondertang_mm_w3", [this] (offs_t o, u8 &d, u8 m) { bank_w<3>(o, d, m); });
}

void msx_cart_wondertang_mm_device::device_reset()
{
	m_bank[0]->set_entry(3);
	m_bank[1]->set_entry(2);
	m_bank[2]->set_entry(1);
	m_bank[3]->set_entry(0);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_16K_RAM,      msx_cart_interface, msx_cart_16k_ram_device,      "msx_cart_16k_ram",      "Generic MSX 16K RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_32K_RAM,      msx_cart_interface, msx_cart_32k_ram_device,      "msx_cart_32k_ram",      "Generic MSX 32K RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_48K_RAM,      msx_cart_interface, msx_cart_48k_ram_device,      "msx_cart_48k_ram",      "Generic MSX 48K RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_64K_RAM,      msx_cart_interface, msx_cart_64k_ram_device,      "msx_cart_64k_ram",      "Generic MSX 64K RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_256K_MM_RAM,  msx_cart_interface, msx_cart_256k_mm_ram_device,  "msx_cart_256k_mm_ram",  "Generic MSX 256K MM RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_512K_MM_RAM,  msx_cart_interface, msx_cart_512k_mm_ram_device,  "msx_cart_512k_mm_ram",  "Generic MSX 512K MM RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_768K_MM_RAM,  msx_cart_interface, msx_cart_768k_mm_ram_device,  "msx_cart_768k_mm_ram",  "Generic MSX 768K MM RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_1024K_MM_RAM, msx_cart_interface, msx_cart_1024k_mm_ram_device, "msx_cart_1024k_mm_ram", "Generic MSX 1024K MM RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_2048K_MM_RAM, msx_cart_interface, msx_cart_2048k_mm_ram_device, "msx_cart_2048k_mm_ram", "Generic MSX 2048K MM RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_4096K_MM_RAM, msx_cart_interface, msx_cart_4096k_mm_ram_device, "msx_cart_4096k_mm_ram", "Generic MSX 4096K MM RAM Expansion")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_DOUBLE_RAM,   msx_cart_interface, msx_cart_double_ram_device,   "msx_cart_double_ram",   "Tecnobytes Double RAM")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MMM,          msx_cart_interface, msx_cart_mmm_device,          "msx_cart_mmm",          "Popolon Musical Memory Mapper")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_WONDERTANG_MEGARAM, msx_cart_interface, msx_cart_wondertang_megaram_device, "msx_cart_wondertang_megaram", "WonderTANG New Juice 2M Super-MegaRAM")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_WONDERTANG_MM, msx_cart_interface, msx_cart_wondertang_mm_device, "msx_cart_wondertang_mm", "WonderTANG New Juice 4M Memory Mapper")
