/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: Mapper 028 implementation.
 */


#pragma once

#include "../LSNLSpiroNes.h"
#include "LSNMapperBase.h"

namespace lsn {

	/**
	 * Class CMapper028
	 * \brief Mapper 028 implementation.
	 *
	 * Description: Mapper 028 implementation.
	 */
	class CMapper028 : public CMapperBase {
	public :
		CMapper028() {
		}
		virtual ~CMapper028() {
		}


		// == Enumerations.
		/** Registers. */
		enum LSN_REG {
			LSN_R_00,
			LSN_R_01,
			LSN_R_80,
			LSN_R_81,
		};


		// == Functions.
		/**
		 * Gets the PGM bank size.
		 *
		 * \return Returns the size of the PGM banks.
		 */
		static constexpr uint16_t						PgmBankSize() { return 32 * 1024; }

		/**
		 * Gets the CHR bank size.
		 *
		 * \return Returns the size of the CHR banks.
		 */
		static constexpr uint16_t						ChrBankSize() { return 8 * 1024; }

		/**
		 * Initializes the mapper with the ROM data.  This is usually to allow the mapper to extract information such as the number of banks it has, as well as make copies of any data it needs to run.
		 *
		 * \param _rRom The ROM data.
		 * \param _pcbCpuBase A pointer to the CPU.
		 */
		virtual void									InitWithRom( LSN_ROM &_rRom, CCpuBase * _pcbCpuBase, CBussable * _pbPpuBus ) {
			CMapperBase::InitWithRom( _rRom, _pcbCpuBase, _pbPpuBus );
			SanitizeRegs<PgmBankSize(), ChrBankSize()>();
		}

		/**
		 * Applies mapping to the CPU and PPU busses.
		 *
		 * \param _pbCpuBus A pointer to the CPU bus.
		 * \param _pbPpuBus A pointer to the PPU bus.
		 */
		virtual void									ApplyMap( CCpuBus * _pbCpuBus, CPpuBus * _pbPpuBus ) {
			CMapperBase::ApplyMap( _pbCpuBus, _pbPpuBus );

			// ================
			// FIXED BANKS
			// ================
			// Set the reads of the fixed bank at the end.		
			m_stFixedOffset = std::max<size_t>( m_prRom->vPrgRom.size(), PgmBankSize() ) - PgmBankSize();
			for ( uint32_t I = 0xC000; I < 0x10000; ++I ) {
				_pbCpuBus->SetReadFunc( uint16_t( I ), &CMapperBase::PgmBankRead_Fixed, this, uint16_t( (I - 0xC000) % m_prRom->vPrgRom.size() ) );
			}
			

			// ================
			// SWAPPABLE BANKS
			// ================
			// CPU.
			for ( uint32_t I = 0x8000; I < 0xC000; ++I ) {
				_pbCpuBus->SetReadFunc( uint16_t( I ), &CMapperBase::PgmBankRead<0, PgmBankSize()>, this, uint16_t( I - 0x8000 ) );
			}
			// PPU.
			for ( uint32_t I = 0x0000; I < 0x2000; ++I ) {
				_pbPpuBus->SetReadFunc( uint16_t( I ), &CMapperBase::ChrBankRead<0, ChrBankSize()>, this, uint16_t( I - 0x0000 ) );
			}


			// ================
			// BANK-SELECT
			// ================
			// Register select.
			for ( uint32_t I = 0x5000; I < 0x6000; ++I ) {
				_pbCpuBus->SetWriteFunc( uint16_t( I ), &CMapper028::SelectBank5000_5FFF, this, 0 );
			}
			// Register value.
			for ( uint32_t I = 0x8000; I < 0x10000; ++I ) {
				_pbCpuBus->SetWriteFunc( uint16_t( I ), &CMapper028::SelectBank8000_FFFF, this, 0 );
			}


			// ================
			// MIRRORING
			// ================
			ApplyControllableMirrorMap( _pbPpuBus );
		}


	protected :
		// == Members.
		LSN_REG											m_rReg;										/**< Which register to use. */
		uint8_t											m_ui8Reg00;									/**< Register $00. */
		uint8_t											m_ui8Reg80;									/**< Register $80. */
		uint8_t											m_ui8Reg01;									/**< Register $01. */
		uint8_t											m_ui8Reg81;									/**< Register $81. */
		uint8_t											m_ui8Mode;									/**< The mode value. */


		// == Functions.
		/**
		 * Updates the mode value.
		 * 
		 * \param _ui8Val The update value.
		 **/
		void inline 									UpdateMode( uint8_t _ui8Val ) {
			if ( (m_ui8Mode & 2) == 0 ) {
				m_ui8Mode &= 0xFE;
				m_ui8Mode |= _ui8Val >> 4 & 1;
			}
			ApplyMirror();
		}

		/**
		 * Applies mirroring.
		 **/
		void inline										ApplyMirror() {
			switch ( m_ui8Mode & 0b11 ) {
				case 0 : {
					m_mmMirror = LSN_MM_1_SCREEN_A;
					break;
				}
				case 1 : {
					m_mmMirror = LSN_MM_1_SCREEN_B;
					break;
				}
				case 2 : {
					m_mmMirror = LSN_MM_VERTICAL;
					break;
				}
				case 3 : {
					m_mmMirror = LSN_MM_HORIZONTAL;
					break;
				}
			}
		}

		/**
		 * Selects a register.
		 *
		 * \param _pvParm0 A data value assigned to this address.
		 * \param _ui16Parm1 A 16-bit parameter assigned to this address.  Typically this will be the address to write to _pui8Data.
		 * \param _pui8Data The buffer to which to write.
		 * \param _ui8Val The value to write.
		 */
		static void LSN_FASTCALL						SelectBank5000_5FFF( void * _pvParm0, uint16_t /*_ui16Parm1*/, uint8_t * /*_pui8Data*/, uint8_t _ui8Val ) {
			CMapper028 * pmThis = reinterpret_cast<CMapper028 *>(_pvParm0);
			switch ( _ui8Val & 0x81 ) {
				case 0x00 : { pmThis->m_rReg = LSN_R_00; break; }
				case 0x01 : { pmThis->m_rReg = LSN_R_01; break; }
				case 0x80 : { pmThis->m_rReg = LSN_R_80; break; }
				case 0x81 : { pmThis->m_rReg = LSN_R_81; break; }
			}
		}

		/**
		 * Selects a bank.
		 *
		 * \param _pvParm0 A data value assigned to this address.
		 * \param _ui16Parm1 A 16-bit parameter assigned to this address.  Typically this will be the address to write to _pui8Data.
		 * \param _pui8Data The buffer to which to write.
		 * \param _ui8Val The value to write.
		 */
		static void LSN_FASTCALL						SelectBank8000_FFFF( void * _pvParm0, uint16_t /*_ui16Parm1*/, uint8_t * /*_pui8Data*/, uint8_t _ui8Val ) {
			CMapper028 * pmThis = reinterpret_cast<CMapper028 *>(_pvParm0);
			switch ( pmThis->m_rReg ) {
				case 0x00 : {
					/*	7654 3210
					 *	   M   BB
					 *	   |   ++- Set CHR RAM A14-A13
					 *	   +------ Set mirroring mode bit 0 if H/V mirroring is disabled
					 */
					pmThis->m_ui8Reg00 = _ui8Val;
					pmThis->SetChrBank<0, ChrBankSize()>( _ui8Val & 0b11 );
					pmThis->UpdateMode( _ui8Val );
					break;
				}
				case 0x01 : {
					/*	7654 3210
					 *	   M BBBB
					 *	   | ++++- Set current PRG ROM bank
					 *	   +------ Set mirroring mode bit 0 if H/V mirroring is disabled
					 */
					pmThis->m_ui8Reg01 = _ui8Val;
					pmThis->UpdateMode( _ui8Val );
					break;
				}
				case 0x80 : {
					/*	7654 3210
					 *	  SS PPMM
					 *	  || ||++- Nametable mirroring mode
					 *	  || ++--- PRG bank mode
					 *	  ++------ PRG outer bank size
					 */
					pmThis->m_ui8Reg80 = _ui8Val;
					pmThis->m_ui8Mode = _ui8Val & 0x3F;
					pmThis->ApplyMirror();
					break;
				}
				case 0x81 : {
					/*	7654 3210
					 *	BBBB Bbbb
					 *	++++-++++- Set outer PRG ROM bank
					 */
					pmThis->m_ui8Reg81 = _ui8Val;
					pmThis->m_ui8Mode = _ui8Val & 0x3F;
					pmThis->ApplyMirror();
					break;
				}
			}
		}
	};

}	// namespace lsn
