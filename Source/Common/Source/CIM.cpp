#include <CIM.hpp>
#include <Utility.hpp>
#include <iostream>
#include <iomanip>

namespace cim2gdi
{
	CIMFile::CIMFile( )
	{
	}

	CIMFile::~CIMFile( )
	{
		this->Close( );
	}

	int CIMFile::Open( const std::string &p_FileName )
	{
		m_File = CreateFile( p_FileName.c_str( ), GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL );
		
		if( m_File == INVALID_HANDLE_VALUE )
		{
			std::cout << "Failed to open file for reading" << std::endl;

			return 1;
		}

		return 0;
	}

	void CIMFile::Close( )
	{
		if( m_File != INVALID_HANDLE_VALUE )
		{
			CloseHandle( m_File );
			m_File = INVALID_HANDLE_VALUE;
		}
	}

	int CIMFile::Parse( )
	{
		DWORD BytesRead;

		CIM_HEADER CIMHeader;				

		ReadFile( m_File, &CIMHeader, sizeof( CIMHeader ), &BytesRead,
			NULL );

		char IDString[ 5 ];
		ZeroMemory( IDString, sizeof( IDString ) );
		memcpy( IDString, CIMHeader.ID, sizeof( CIMHeader.ID ) );

		char TypeString[ 5 ];
		ZeroMemory( TypeString, sizeof( TypeString ) );
		memcpy( TypeString, CIMHeader.ImageType,
			sizeof( CIMHeader.ImageType ) );

		CIMHeader.FileSize = SwapUInt32( CIMHeader.FileSize );
		CIMHeader.SDAFlags = SwapUInt32( CIMHeader.SDAFlags );
		CIMHeader.HDAFlags = SwapUInt32( CIMHeader.HDAFlags );
		CIMHeader.SDATOC = SwapUInt32( CIMHeader.SDATOC );
		CIMHeader.SDADisc = SwapUInt32( CIMHeader.SDADisc );
		CIMHeader.HDATOC = SwapUInt32( CIMHeader.HDATOC );
		CIMHeader.HDADisc = SwapUInt32( CIMHeader.HDADisc );

		std::cout << "CIM Header Information" << std::endl;
		std::cout << "\tID:        " << IDString << std::endl;		
		std::cout << "\tFile size: 0x" << std::hex << CIMHeader.FileSize <<
			std::dec << std::endl;
		std::cout << "\tType:      " << TypeString << std::endl;
		std::cout << std::setfill( '0' ) << std::hex;
		std::cout << "\tSDA Flags: 0x" << std::setw( 8 ) <<
			CIMHeader.SDAFlags << std::endl;
		std::cout << "\tHDA Flags: 0x" << std::setw( 8 ) <<
			CIMHeader.HDAFlags << std::endl;
		std::cout << "\tSDA TOC:   0x" << std::setw( 8 ) <<
			CIMHeader.SDATOC << std::endl;
		std::cout << "\tSDA Disc:  0x" << std::setw( 8 ) <<
			CIMHeader.SDADisc << std::endl;
		std::cout << "\tHDA TOC:   0x" << std::setw( 8 ) <<
			CIMHeader.HDATOC << std::endl;
		std::cout << "\tHDA Disc:  0x" << std::setw( 8 ) <<
			CIMHeader.HDADisc << std::endl;
		std::cout << std::setfill( ' ' ) << std::setw( 0 ) << std::dec;

		// Jump to the SDA TOC
		SetFilePointer( m_File, CIMHeader.SDATOC, NULL, FILE_BEGIN );

		SDA_TOC SDATOC;

		ReadFile( m_File, &SDATOC, sizeof( SDATOC ), &BytesRead,
			NULL );

		ZeroMemory( IDString, sizeof( IDString ) );
		memcpy( IDString, SDATOC.ID, sizeof( SDATOC.ID ) );

		SDATOC.SDADisc = SwapUInt32( SDATOC.SDADisc );

		std::cout << "Single Density Area TOC Information" << std::endl;

		std::cout << "\tID:   " << IDString << std::endl;

		std::cout << std::setfill( '0' ) << std::hex;
		std::cout << "\tDisc: 0x" << std::setw( 8 ) <<
			SDATOC.SDADisc << std::endl;
		std::cout << std::setfill( ' ' ) << std::dec;

		// Keep reading track entries until the lead out is found
		bool Track = true;

		std::cout << "\tTrack Information" << std::endl;

		do
		{
			TRACK_ENTRY TrackEntry;

			ReadFile( m_File, &TrackEntry, sizeof( TrackEntry ),
				&BytesRead, NULL );
			
			std::string TrackType = TrackEntry.Control == 0x41 ?
				"DATA" : "AUDIO";
			std::string TrackForm;
			
			if( TrackEntry.Form == 0x02 )
			{
				TrackForm = "MODE1";
			}
			if( TrackEntry.Form == 0x00 )
			{
				TrackForm = "CCDA";
			}

			std::cout << "\t\t" << TrackType << " ";

			if( ( TrackEntry.TrackNumber == 0x00 ) &&
				( TrackEntry.TrackNumber == 0x00 ) )
			{
				std::cout << "LeadIn";
			}
			else if( ( TrackEntry.TrackNumber == 0xAA ) &&
				( TrackEntry.Transform == 0x01 ) )
			{
				std::cout << "LeadOut";
				Track = false;
			}
			else
			{
				std::cout <<+TrackEntry.TrackNumber << "/" <<
					+TrackEntry.Transform;
			}

			std::cout << " " << +TrackEntry.Hours << ":" <<
				( TrackEntry.Minutes & 0x0F ) +
				( ( ( TrackEntry.Minutes & 0xF0 ) >> 4 ) * 10 ) <<
				":" <<
				( TrackEntry.Seconds & 0x0F ) +
				( ( ( TrackEntry.Seconds & 0xF0 ) >> 4 ) * 10 ) <<
				":" << 
				( TrackEntry.Frames & 0x0F ) +
				( ( ( TrackEntry.Frames & 0xF0 ) >> 4 ) * 10 ) << " ";

			unsigned int LSN = ( ( ( TrackEntry.Minutes & 0xF ) +
				( ( ( TrackEntry.Minutes & 0xF0 ) >> 4 ) * 10 ) * 60 * 75 ) +
				( ( TrackEntry.Seconds & 0x0F ) +
				( ( ( TrackEntry.Seconds & 0xF0 ) >> 4 ) * 10 ) ) * 75 ) +
				( TrackEntry.Frames & 0x0F ) +
				( ( ( TrackEntry.Frames & 0xF0 ) >> 4 ) * 10 );

			std::cout << "LSN: " << LSN << std::endl;
		}while( Track );

		// Get the SDA Disc
		SetFilePointer( m_File, CIMHeader.SDADisc, NULL, FILE_BEGIN );

		SDA_DISC SDADisc;
		ReadFile( m_File, &SDADisc, sizeof( SDADisc ), &BytesRead, NULL );

		ZeroMemory( IDString, sizeof( IDString ) );
		memcpy( IDString, SDADisc.ID, sizeof( SDADisc.ID ) );

		SDADisc.Size = SwapUInt32( SDADisc.Size );

		std::cout << "Single Density Area Disc Information" << std::endl;
		std::cout << "\tID:   " << IDString << std::endl;
		std::cout << std::hex << std::setfill( '0' );
		std::cout << "\tSize: 0x" << std::setw( 8 ) << SDADisc.Size <<
			std::endl;
		std::cout << std::dec << std::setfill( ' ' );

		return 0;
	}
}
