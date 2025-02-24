/**
 * Copyright L. Spiro 2023
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: A standard 24-bit RGB filter.
 */

#pragma once

#include "../LSNLSpiroNes.h"
#include "LSNFilterBase.h"
#include "nes_ntsc/nes_ntsc.h"
#include <vector>


namespace lsn {

	/**
	 * Class CRgb24Filter
	 * \brief A standard 24-bit RGB filter.
	 *
	 * Description: A standard 24-bit RGB filter.
	 */
	class CRgb24Filter : public CFilterBase {
	public :
		CRgb24Filter();
		virtual ~CRgb24Filter();


		// == Functions.
		/**
		 * Sets the basic parameters for the filter.
		 *
		 * \param _stBuffers The number of render targets to create.
		 * \param _ui16Width The console screen width.  Typically 256.
		 * \param _ui16Height The console screen height.  Typically 240.
		 * \return Returns the input format requested of the PPU.
		 */
		virtual CDisplayClient::LSN_PPU_OUT_FORMAT			Init( size_t _stBuffers, uint16_t _ui16Width, uint16_t _ui16Height );

		/**
		 * Gets the PPU output format.
		 *
		 * \return Returns the output format from the PPU/input format for this filter.
		 */
		virtual CDisplayClient::LSN_PPU_OUT_FORMAT			InputFormat() const { return CDisplayClient::LSN_POF_RGB32; }

		/**
		 * Gets the bits-per-pixel of the final output.  Will be 16, 24, or 32.
		 *
		 * \return Returns the bits-per-pixel of the final output.
		 */
		virtual uint32_t									OutputBits() const { return 32; }

		/**
		 * Gets a pointer to the output buffer.
		 *
		 * \return Returns a pointer to the output buffer.
		 */
		virtual uint8_t *									OutputBuffer() { return CurTarget(); }

		/**
		 * Tells the filter that rendering to the source buffer has completed and that it should filter the results.  The final buffer, along with
		 *	its width, height, bit-depth, and stride, are returned.
		 *
		 * \param _pui8Input The buffer to be filtered, which will be a pointer to one of the buffers returned by OutputBuffer() previously.  Its format will be that returned in InputFormat().
		 * \param _ui32Width On input, this is the width of the buffer in pixels.  On return, it is filled with the final width, in pixels, of the result.
		 * \param _ui32Height On input, this is the height of the buffer in pixels.  On return, it is filled with the final height, in pixels, of the result.
		 * \param _ui16BitDepth On input, this is the bit depth of the buffer.  On return, it is filled with the final bit depth of the result.
		 * \param _ui32Stride On input, this is the stride of the buffer.  On return, it is filled with the final stride, in bytes, of the result.
		 * \param _ui64PpuFrame The PPU frame associated with the input data.
		 * \param _ui64RenderStartCycle The cycle at which rendering of the first pixel began.
		 * \return Returns a pointer to the filtered output buffer.
		 */
		virtual uint8_t *									ApplyFilter( uint8_t * _pui8Input, uint32_t &/*_ui32Width*/, uint32_t &/*_ui32Height*/, uint16_t &/*_ui16BitDepth*/, uint32_t &/*_ui32Stride*/, uint64_t /*_ui64PpuFrame*/, uint64_t /*_ui64RenderStartCycle*/ );

	protected :
		// == Members.
		
	};

}	// namespace lsn
