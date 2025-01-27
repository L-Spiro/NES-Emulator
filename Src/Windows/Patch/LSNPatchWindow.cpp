#ifdef LSN_USE_WINDOWS

/**
 * Copyright L. Spiro 2025
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: The patch window.
 */

#include "LSNPatchWindow.h"
#include "../../Crc/LSNCrc.h"
#include "../../File/LSNStdFile.h"
#include "../../File/LSNZipFile.h"
#include "../../Localization/LSNLocalization.h"
#include "../../System/LSNSystemBase.h"
#include "../WinUtilities/LSNWinUtilities.h"

#include <commdlg.h>
#include <filesystem>
#include <set>


namespace lsn {

	CPatchWindow::CPatchWindow( const LSW_WIDGET_LAYOUT &_wlLayout, CWidget * _pwParent, bool _bCreateWidget, HMENU _hMenu, uint64_t _ui64Data ) :
		lsw::CMainWindow( _wlLayout, _pwParent, _bCreateWidget, _hMenu, _ui64Data ),
		m_poOptions( reinterpret_cast<LSN_OPTIONS *>(_ui64Data) ),
		m_bOutIsAutoFilled( false ) {
	}
	CPatchWindow::~CPatchWindow() {
	}

	// == Functions.
	/**
	 * The WM_INITDIALOG handler.
	 *
	 * \return Returns an LSW_HANDLED code.
	 */
	CWidget::LSW_HANDLED CPatchWindow::InitDialog() {

		return CMainWindow::InitDialog();
	}

	/**
	 * The WM_CLOSE handler.
	 *
	 * \return Returns an LSW_HANDLED code.
	 */
	CWidget::LSW_HANDLED CPatchWindow::Close() {
		//::DestroyWindow( Wnd() );
		if ( m_pwParent ) {
			::PostMessageW( m_pwParent->Wnd(), CWinUtilities::LSN_CLOSE_PATCHER, 0, 0 );
		}
		::EndDialog( Wnd(), 0 );
		return LSW_H_CONTINUE;
	}

	/**
	 * Handles the WM_COMMAND message.
	 *
	 * \param _wCtrlCode 0 = from menu, 1 = from accelerator, otherwise it is a Control-defined notification code.
	 * \param _wId The ID of the control if _wCtrlCode is not 0 or 1.
	 * \param _pwSrc The source control if _wCtrlCode is not 0 or 1.
	 * \return Returns an LSW_HANDLED code.
	 */
	CWidget::LSW_HANDLED CPatchWindow::Command( WORD _wCtrlCode, WORD _wId, CWidget * _pwSrc ) {
		switch ( _wId ) {
			case CPatchWindowLayout::LSN_PWI_FILE_IN_BUTTON : {
				OPENFILENAMEW ofnOpenFile = { sizeof( ofnOpenFile ) };
				std::u16string szFileName;
				szFileName.resize( 0xFFFF + 2 );

				ofnOpenFile.hwndOwner = Wnd();
				ofnOpenFile.lpstrFilter = LSN_LSTR( LSN_ALL_SUPPORTED_FILES___NES____ZIP____NES___ZIP_ );
				ofnOpenFile.lpstrFile = reinterpret_cast<LPWSTR>(&szFileName[0]);
				ofnOpenFile.nMaxFile = DWORD( szFileName.size() );
				ofnOpenFile.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
				ofnOpenFile.lpstrInitialDir = m_poOptions->wInRomInitPath.c_str();
	
				std::u16string u16Path, u16FileName;
				if ( ::GetOpenFileNameW( &ofnOpenFile ) ) {
					m_poOptions->wInRomInitPath = std::filesystem::path( ofnOpenFile.lpstrFile ).remove_filename();

					lsn::CZipFile zfFile;
					if ( zfFile.Open( reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile) ) && zfFile.IsArchive() ) {
						std::vector<std::u16string> vFiles;
						zfFile.GatherArchiveFiles( vFiles, u".nes" );

						if ( vFiles.size() == 1 ) {
							u16Path = reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile);
							u16Path += u'{';
							u16Path += vFiles[0];
							u16Path += u'}';
							u16FileName = std::filesystem::path( vFiles[0] ).filename().u16string();
						}
					}
					else if ( !zfFile.IsArchive() ) {
						u16Path = reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile);
						u16FileName = std::filesystem::path( ofnOpenFile.lpstrFile ).filename().u16string();
					}
					auto pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_FILE_IN_EDIT );
					if ( pwPathEdit ) {
						pwPathEdit->SetTextW( reinterpret_cast<const wchar_t *>(u16Path.c_str()) );
					}
					
				}
				break;
			}
			case CPatchWindowLayout::LSN_PWI_FILE_PATCH_BUTTON : {
				OPENFILENAMEW ofnOpenFile = { sizeof( ofnOpenFile ) };
				std::u16string szFileName;
				szFileName.resize( 0xFFFF + 2 );

				ofnOpenFile.hwndOwner = Wnd();
				ofnOpenFile.lpstrFilter = LSN_LSTR( LSN_PATCH_FILE_TYPES );
				ofnOpenFile.lpstrFile = reinterpret_cast<LPWSTR>(&szFileName[0]);
				ofnOpenFile.nMaxFile = DWORD( szFileName.size() );
				ofnOpenFile.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
				ofnOpenFile.lpstrInitialDir = m_poOptions->wPatchInitPath.c_str();
				
				std::u16string u16Path;
				if ( ::GetOpenFileNameW( &ofnOpenFile ) ) {
					m_poOptions->wPatchInitPath = std::filesystem::path( ofnOpenFile.lpstrFile ).remove_filename();
					size_t sBpsStart = 1, sTxtStart = 1;
					lsn::CZipFile zfFile;
					if ( zfFile.Open( reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile) ) && zfFile.IsArchive() ) {
						std::vector<std::u16string> vFiles;
						std::vector<std::vector<uint8_t>> vPatchImage;
						zfFile.ExtractToMemory( vFiles, vPatchImage, u".ips" );
						sBpsStart = vFiles.size();
						zfFile.ExtractToMemory( vFiles, vPatchImage, u".bps" );
						sTxtStart = vFiles.size();
						zfFile.ExtractToMemory( vFiles, vPatchImage, u".txt" );
						zfFile.ExtractToMemory( vFiles, vPatchImage, u".md" );

						//if ( vFiles.size() == 1 ) {
						//	m_vPatchRomFile.clear();
						//	if ( !zfFile.ExtractToMemory( vFiles[0], m_vPatchRomFile ) ) {
						//	}
						//	u16Path = reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile);
						//	u16Path += u'{';
						//	u16Path += vFiles[0];
						//	u16Path += u'}';
						//	/*CUtilities::LSN_FILE_PATHS fpPath;
						//	CUtilities::DeconstructFilePath( reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile), fpPath );*/
						//	//fpPath.u16sFile
						//}
						try {
							m_vPatchInfo.resize( vFiles.size() );
							for ( size_t I = 0; I < vFiles.size(); ++I ) {
								m_vPatchInfo[I].u16FullPath = reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile);
								m_vPatchInfo[I].u16FullPath += u'{' + vFiles[I] + u'}';
								m_vPatchInfo[I].vLoadedPatchFile = std::move( vPatchImage[I] );
								u16Path += m_vPatchInfo[I].u16FullPath;
								u16Path += u';';
							}
						}
						catch ( ... ) {
						}
						
					}
					else if ( !zfFile.IsArchive() ) {
						lsn::CStdFile sfFile;
						if ( sfFile.Open( reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile) ) ) {
							try {
								m_vPatchInfo.resize( 1 );
								m_vPatchInfo[0].u16FullPath = reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile);
							}
							catch ( ... ) {
							}
							if ( !sfFile.LoadToMemory( m_vPatchInfo[0].vLoadedPatchFile ) ) {
							}
							u16Path = reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile);
						}
					}

					try {
						//std::set<std::u16string> sPaths;
						for ( size_t I = 0; I < m_vPatchInfo.size(); ++I ) {
							m_vPatchInfo[I].ui32PatchCrc = CCrc::GetCrc( m_vPatchInfo[I].vLoadedPatchFile.data(), m_vPatchInfo[I].vLoadedPatchFile.size() );
							if ( I < sTxtStart ) {
								if ( m_vPatchInfo[I].vLoadedPatchFile.size() >= 4 ) {
									m_vPatchInfo[I].ui32PatchCrcMinus4 = CCrc::GetCrc( m_vPatchInfo[I].vLoadedPatchFile.data(), m_vPatchInfo[I].vLoadedPatchFile.size() - 4 );
								}
							}
							if ( I >= sBpsStart && I < sTxtStart ) {
								if ( m_vPatchInfo[I].vLoadedPatchFile.size() >= 12 ) {
									m_vPatchInfo[I].ui32Crc = (*reinterpret_cast<uint32_t *>(&m_vPatchInfo[I].vLoadedPatchFile[m_vPatchInfo[I].vLoadedPatchFile.size()-12]));
								}
							}

							::OutputDebugStringW( reinterpret_cast<const wchar_t *>((m_vPatchInfo[I].u16FullPath + u"\r\n").c_str()) );
						}


						std::set<std::u16string> sPaths;
						for ( size_t I = 0; I < m_vPatchInfo.size(); ++I ) {
							CUtilities::LSN_FILE_PATHS fpPath;
							auto u16Tmp = m_vPatchInfo[I].u16FullPath;
							u16Tmp = CUtilities::Replace( u16Tmp, std::u16string( u"{" ), std::u16string( u"\\" ) );
							u16Tmp = CUtilities::Replace( u16Tmp, std::u16string( u"}" ), std::u16string( u"" ) );
							u16Tmp = CUtilities::Replace( u16Tmp, u'/', u'\\' );
							CUtilities::DeconstructFilePath( u16Tmp.c_str(), fpPath );
							sPaths.insert( fpPath.u16sPath );
						}
						if ( !sPaths.size() ) {
							sPaths.insert( u"" );
						}
					}
					catch ( ... ) {
					}
					auto pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_FILE_PATCH_EDIT );
					if ( pwPathEdit ) {
						pwPathEdit->SetTextW( reinterpret_cast<const wchar_t *>(u16Path.c_str()) );
						//::OutputDebugStringW( reinterpret_cast<const wchar_t *>((u16Path + u"\r\n").c_str()) );
					}
				}
				break;
			}
			case CPatchWindowLayout::LSN_PWI_FILE_OUT_BUTTON : {
				OPENFILENAMEW ofnOpenFile = { sizeof( ofnOpenFile ) };
				std::u16string szFileName;
				szFileName.resize( 0xFFFF + 2 );

				ofnOpenFile.hwndOwner = Wnd();
				ofnOpenFile.lpstrFilter = LSN_LSTR( LSN_NES_FILES____NES____NES_ );
				ofnOpenFile.lpstrFile = reinterpret_cast<LPWSTR>(&szFileName[0]);
				ofnOpenFile.nMaxFile = DWORD( szFileName.size() );
				ofnOpenFile.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
				ofnOpenFile.lpstrInitialDir = m_poOptions->wOutRomInitPath.c_str();
	
				std::u16string u16Path, u16FileName;
				if ( ::GetSaveFileNameW( &ofnOpenFile ) ) {
					m_poOptions->wOutRomInitPath = std::filesystem::path( ofnOpenFile.lpstrFile ).remove_filename();

					u16Path = reinterpret_cast<const char16_t *>(ofnOpenFile.lpstrFile);
					u16FileName = std::filesystem::path( ofnOpenFile.lpstrFile ).filename().u16string();
					auto pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_FILE_OUT_EDIT );
					if ( pwPathEdit ) {
						pwPathEdit->SetTextW( reinterpret_cast<const wchar_t *>(u16Path.c_str()) );
						m_bOutIsAutoFilled = false;
					}
				}
				break;
			}
			case CPatchWindowLayout::LSN_PWI_FILE_IN_EDIT : {
				if ( _wCtrlCode == EN_CHANGE ) {
					auto wsText = _pwSrc->GetTextW();
					CUtilities::LSN_FILE_PATHS fpPath;
					CUtilities::DeconstructFilePath( reinterpret_cast<const char16_t *>(wsText.c_str()), fpPath );
					if ( CUtilities::LastChar( fpPath.u16sFullPath ) == u'}' ) {
						lsn::CZipFile zfFile;
						auto aZipPath = fpPath.u16sPath;
						aZipPath.pop_back();
						if ( zfFile.Open( aZipPath.c_str() ) && zfFile.IsArchive() ) {
							m_vPatchRomFile.clear();
							if ( zfFile.ExtractToMemory( fpPath.u16sFile, m_vPatchRomFile ) ) {
							}
						}
					}
					else {
						lsn::CStdFile sfFile;
						if ( sfFile.Open( fpPath.u16sFullPath.c_str() ) ) {
							m_vPatchRomFile.clear();
							if ( sfFile.LoadToMemory( m_vPatchRomFile ) ) {
							}
						}
					}
					m_u16RomPath = fpPath.u16sPath + fpPath.u16sFile;

					auto pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_FILE_OUT_EDIT );
					if ( pwPathEdit ) {
						if ( m_bOutIsAutoFilled || !pwPathEdit->GetTextW().size() ) {
							if ( CUtilities::LastChar( fpPath.u16sFullPath ) == u'}' ) { fpPath.u16sPath.pop_back(); }
							std::filesystem::path pPath( fpPath.u16sPath );
							auto aTmp = pPath.remove_filename();
							aTmp /= fpPath.u16sFile;
							aTmp = aTmp.replace_filename( CUtilities::NoExtension( std::filesystem::path( fpPath.u16sFile ).filename().u16string() ) + u" Patched" );

							auto aFinal = aTmp;
							aFinal += ".nes";
							size_t sIdx = 0;
							while ( std::filesystem::exists( aFinal ) ) {
								aFinal = aTmp;
								aFinal += " " + std::to_string( sIdx++ );
								aFinal += ".nes";
							}
							
							pwPathEdit->SetTextW( aFinal.native().c_str() );
							m_bOutIsAutoFilled = true;
							m_poOptions->wOutRomInitPath = aFinal.remove_filename();
						}
					}
					UpdateInfo();
					return LSW_H_CONTINUE;
				}
				break;
			}
			case CPatchWindowLayout::LSN_PWI_FILE_OUT_EDIT : {
				if ( _wCtrlCode == EN_CHANGE ) {
					m_bOutIsAutoFilled = false;
					return LSW_H_CONTINUE;
				}
				break;
			}
		}
		return LSW_H_CONTINUE;
	}

	/**
	 * Handles the WM_GETMINMAXINFO message.
	 * 
	 * \param _pmmiInfo The min/max info structure to fill out.
	 * \return Returns an LSW_HANDLED code.
	 **/
	CWidget::LSW_HANDLED CPatchWindow::GetMinMaxInfo( MINMAXINFO * _pmmiInfo ) {
		LONG lLeft = 100;
		if ( m_pwParent ) {
			auto aTmp = m_pwParent->FindChild( CPatchWindowLayout::LSN_PWI_FILE_IN_EDIT );
			if ( aTmp ) {
				lLeft = aTmp->WindowRect().ScreenToClient( Wnd() ).left;
			}

			aTmp = m_pwParent->FindChild( CPatchWindowLayout::LSN_PWI_FILE_IN_BUTTON );
			if ( aTmp ) {
				lLeft += aTmp->WindowRect().Width();
			}

			lLeft += 130;
		}
		_pmmiInfo->ptMinTrackSize.x = lLeft;
		_pmmiInfo->ptMinTrackSize.y = _pmmiInfo->ptMaxTrackSize.y = m_rStartingRect.Height();
		return LSW_H_HANDLED;
	}

	/**
	 * Updates the source ROM information labels.
	 **/
	void CPatchWindow::UpdateInfo() {
		LSN_ROM rRom;
		if ( m_vPatchRomFile.size() && CSystemBase::LoadRom( m_vPatchRomFile, rRom, m_u16RomPath ) ) {
			
			auto pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_INFO_CRC_LABEL );
			if ( pwPathEdit ) {
				auto aFullCrc = CCrc::GetCrc( m_vPatchRomFile.data(), m_vPatchRomFile.size() );
				pwPathEdit->SetTextW( std::format( L"Full CRC32:\t{:08X}", aFullCrc ).c_str() );
			}
			pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_INFO_ROM_CRC_LABEL );
			if ( pwPathEdit ) {
				pwPathEdit->SetTextW( std::format( L"ROM CRC32:\t{:08X}", rRom.riInfo.ui32Crc ).c_str() );
			}

			pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_INFO_ROM_PGM_SIZE_LABEL );
			if ( pwPathEdit ) {
				pwPathEdit->SetTextW( std::format( L"ROM PGM Size:\t{0:08X} ({0} bytes, {1:g} kibibytes)", rRom.vPrgRom.size(), rRom.vPrgRom.size() / 1024.0 ).c_str() );
			}
			pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_INFO_ROM_CHR_SIZE_LABEL );
			if ( pwPathEdit ) {
				pwPathEdit->SetTextW( std::format( L"ROM CHR Size:\t{0:08X} ({0} bytes, {1:g} kibibytes)", rRom.vChrRom.size(), rRom.vChrRom.size() / 1024.0 ).c_str() );
			}
			pwPathEdit = FindChild( CPatchWindowLayout::LSN_PWI_INFO_ROM_MIRROR_LABEL );
			if ( pwPathEdit ) {
				switch ( rRom.riInfo.mmMirroring ) {
					case LSN_MM_VERTICAL : {
						pwPathEdit->SetTextW( std::format( L"Mirroring:\tVertical", rRom.vPrgRom.size(), rRom.vPrgRom.size() / 1024.0 ).c_str() );
						break;
					}
					case LSN_MM_HORIZONTAL : {
						pwPathEdit->SetTextW( std::format( L"Mirroring:\tHorizontal", rRom.vPrgRom.size(), rRom.vPrgRom.size() / 1024.0 ).c_str() );
						break;
					}
					case LSN_MM_4_SCREENS : {
						pwPathEdit->SetTextW( std::format( L"Mirroring:\t4 Screens", rRom.vPrgRom.size(), rRom.vPrgRom.size() / 1024.0 ).c_str() );
						break;
					}
					case LSN_MM_1_SCREEN_A : {
						pwPathEdit->SetTextW( std::format( L"Mirroring:\tScreen A", rRom.vPrgRom.size(), rRom.vPrgRom.size() / 1024.0 ).c_str() );
						break;
					}
					case LSN_MM_1_SCREEN_B : {
						pwPathEdit->SetTextW( std::format( L"Mirroring:\tScreen B", rRom.vPrgRom.size(), rRom.vPrgRom.size() / 1024.0 ).c_str() );
						break;
					}
				}
			}
		}
	}

}	// namespace lsn

#endif	// #ifdef LSN_USE_WINDOWS
