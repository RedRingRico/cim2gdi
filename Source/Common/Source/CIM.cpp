#include <CIM.hpp>
#include <Utility.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

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
		this->ParseHeader( );

		DWORD BytesRead;
		char IDString[ 5 ];
		ZeroMemory( IDString, sizeof( IDString ) );

		// Jump to the SDA TOC
		SetFilePointer( m_File, m_CIMHeader.SDATOC, NULL, FILE_BEGIN );

		TOC SDATOC;

		ReadFile( m_File, &SDATOC, sizeof( SDATOC ), &BytesRead,
			NULL );

		ZeroMemory( IDString, sizeof( IDString ) );
		memcpy( IDString, SDATOC.ID, sizeof( SDATOC.ID ) );

		SDATOC.DiscOffset = SwapUInt32( SDATOC.DiscOffset );

		std::cout << "Single Density Area TOC Information" << std::endl;

		std::cout << "\tID:   " << IDString << std::endl;

		std::cout << std::setfill( '0' ) << std::hex;
		std::cout << "\tDisc: 0x" << std::setw( 8 ) <<
			SDATOC.DiscOffset << std::endl;
		std::cout << std::setfill( ' ' ) << std::dec;

		this->ExtractTracks( m_SDA );

		std::cout << "Processed SDA Tracks" << std::endl;

		this->PrintTrackInformation( m_SDA );

		// Get the SDA Disc
		SetFilePointer( m_File, m_CIMHeader.SDADisc, NULL, FILE_BEGIN );

		DISC SDADisc;
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

		// Copy the data from each track in the SDA to track[nm].[raw|bin]
		this->WriteTracks( m_SDA, AREA_SINGLE_DENSITY );

		// Get the HDA information
		SetFilePointer( m_File, m_CIMHeader.HDATOC, NULL, FILE_BEGIN );

		TOC HDATOC;

		ReadFile( m_File, &HDATOC, sizeof( HDATOC ), &BytesRead, NULL );

		ZeroMemory( IDString, sizeof( IDString ) );
		memcpy( IDString, HDATOC.ID, sizeof( HDATOC.ID ) );

		HDATOC.DiscOffset = SwapUInt32( HDATOC.DiscOffset );

		std::cout << "High Density Area TOC Information" << std::endl;

		std::cout << "\tID:   " << IDString << std::endl;

		std::cout << std::setfill( '0' ) << std::hex;
		std::cout << "\tDisc: 0x" << std::setw( 8 ) <<
			SDATOC.DiscOffset << std::endl;
		std::cout << std::setfill( ' ' ) << std::dec;

		this->ExtractTracks( m_HDA );

		std::cout << "Processed HDA Tracks" << std::endl;

		this->PrintTrackInformation( m_HDA );

		// Get the HDA Disc
		SetFilePointer( m_File, m_CIMHeader.HDADisc, NULL, FILE_BEGIN );

		DISC HDADisc;
		ReadFile( m_File, &HDADisc, sizeof( HDADisc ), &BytesRead, NULL );

		ZeroMemory( IDString, sizeof( IDString ) );
		memcpy( IDString, HDADisc.ID, sizeof( HDADisc.ID ) );

		HDADisc.Size = SwapUInt32( HDADisc.Size );

		std::cout << "High Density Area Disc Information" << std::endl;
		std::cout << "\tID:   " << IDString << std::endl;
		std::cout << std::hex << std::setfill( '0' );
		std::cout << "\tSize: 0x" << std::setw( 8 ) << HDADisc.Size <<
			std::endl;
		std::cout << std::dec << std::setfill( ' ' );

		// Copy the data from each track in the HDA to track[nm].[raw|bin]
		this->WriteTracks( m_HDA, AREA_HIGH_DENSITY, m_SDA.size( ) );

		this->WriteGDI( );

		return 0;
	}

	int CIMFile::ParseHeader( )
	{
		DWORD BytesRead;			

		ReadFile( m_File, &m_CIMHeader, sizeof( m_CIMHeader ), &BytesRead,
			NULL );

		char IDString[ 5 ];
		ZeroMemory( IDString, sizeof( IDString ) );
		memcpy( IDString, m_CIMHeader.ID, sizeof( m_CIMHeader.ID ) );

		char TypeString[ 5 ];
		ZeroMemory( TypeString, sizeof( TypeString ) );
		memcpy( TypeString, m_CIMHeader.ImageType,
			sizeof( m_CIMHeader.ImageType ) );

		m_CIMHeader.FileSize = SwapUInt32( m_CIMHeader.FileSize );
		m_CIMHeader.SDAFlags = SwapUInt32( m_CIMHeader.SDAFlags );
		m_CIMHeader.HDAFlags = SwapUInt32( m_CIMHeader.HDAFlags );
		m_CIMHeader.SDATOC = SwapUInt32( m_CIMHeader.SDATOC );
		m_CIMHeader.SDADisc = SwapUInt32( m_CIMHeader.SDADisc );
		m_CIMHeader.HDATOC = SwapUInt32( m_CIMHeader.HDATOC );
		m_CIMHeader.HDADisc = SwapUInt32( m_CIMHeader.HDADisc );

		std::cout << "CIM Header Information" << std::endl;
		std::cout << "\tID:        " << IDString << std::endl;		
		std::cout << "\tFile size: 0x" << std::hex << m_CIMHeader.FileSize <<
			std::dec << std::endl;
		std::cout << "\tType:      " << TypeString << std::endl;
		std::cout << std::setfill( '0' ) << std::hex;
		std::cout << "\tSDA Flags: 0x" << std::setw( 8 ) <<
			m_CIMHeader.SDAFlags << std::endl;
		std::cout << "\tHDA Flags: 0x" << std::setw( 8 ) <<
			m_CIMHeader.HDAFlags << std::endl;
		std::cout << "\tSDA TOC:   0x" << std::setw( 8 ) <<
			m_CIMHeader.SDATOC << std::endl;
		std::cout << "\tSDA Disc:  0x" << std::setw( 8 ) <<
			m_CIMHeader.SDADisc << std::endl;
		std::cout << "\tHDA TOC:   0x" << std::setw( 8 ) <<
			m_CIMHeader.HDATOC << std::endl;
		std::cout << "\tHDA Disc:  0x" << std::setw( 8 ) <<
			m_CIMHeader.HDADisc << std::endl;
		std::cout << std::setfill( ' ' ) << std::setw( 0 ) << std::dec;

		return 0;
	}

	int CIMFile::ExtractTracks( std::vector< TRACK > &p_Area )
	{
		DWORD BytesRead;
		unsigned int TrackCounter = 0UL;

		TRACK CurrentTrack, PreviousTrack;
		ZeroMemory( &CurrentTrack, sizeof( CurrentTrack ) );
		ZeroMemory( &PreviousTrack, sizeof( PreviousTrack ) );

		// Keep reading track entries until the lead out is found
		bool Track = true;

		std::cout << "\tTrack Information" << std::endl;

		do
		{
			TRACK_ENTRY TrackEntry;

			ReadFile( m_File, &TrackEntry, sizeof( TrackEntry ),
				&BytesRead, NULL );

			++TrackCounter;

			unsigned int LSN =
				( ( ( TrackEntry.Minutes & 0xF ) +
					( ( ( TrackEntry.Minutes & 0xF0 ) >> 4 ) * 10 ) )
						* 60 * 75 ) +
				( ( ( TrackEntry.Seconds & 0x0F ) +
					( ( ( TrackEntry.Seconds & 0xF0 ) >> 4 ) * 10 ) ) * 75 ) +
				( TrackEntry.Frames & 0x0F ) +
					( ( ( TrackEntry.Frames & 0xF0 ) >> 4 ) * 10 );

			// The nn/00 track seems to be the beginning of the actual track
			// while nn/01 appears to indicate the offset to the data
			if( TrackEntry.Transform == 0 )
			{
				/*if( NextTrackFirst == true )
				{
					TrackOffset = LSN;
					NextTrackFirst = false;
				}*/

				PreviousTrack = CurrentTrack;
				ZeroMemory( &CurrentTrack, sizeof( CurrentTrack ) );

				CurrentTrack.StartAddress = LSN;

				if( PreviousTrack.PhysicalAddress != 0 )
				{
					PreviousTrack.Size = CurrentTrack.StartAddress -
						PreviousTrack.PhysicalAddress;

					p_Area.push_back( PreviousTrack );
				}
			}
			if( TrackEntry.Transform == 1 )
			{
				if( TrackEntry.TrackNumber != 0xAA )
				{					
					CurrentTrack.PhysicalAddress = LSN;
				}
			}
			
			std::string TrackType = TrackEntry.Control == 0x41 ?
				"DATA" : "AUDIO";
			std::string TrackForm;
			
			if( TrackEntry.Form == 0x02 )
			{
				TrackForm = "MODE1";
				CurrentTrack.Type = TRACK_TYPE_MODE1;
			}
			if( TrackEntry.Form == 0x00 )
			{
				TrackForm = "CCDA";
				CurrentTrack.Type = TRACK_TYPE_CCDA;
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

				PreviousTrack = CurrentTrack;
				ZeroMemory( &CurrentTrack, sizeof( CurrentTrack ) );

				CurrentTrack.StartAddress = LSN;

				if( PreviousTrack.PhysicalAddress != 0 )
				{
					PreviousTrack.Size = CurrentTrack.StartAddress -
						PreviousTrack.PhysicalAddress;

					p_Area.push_back( PreviousTrack );
				}

				// Two tracks are used for 16-byte alignment
				if( ( TrackCounter % 2 ) != 0 )
				{
					// Skip over a track not read in
					SetFilePointer( m_File, 8, NULL, FILE_CURRENT );
				}
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
			

			std::cout << "LSN: " << LSN << std::endl;
		}while( Track );

		return 0;
	}

	void CIMFile::PrintTrackInformation( const std::vector< TRACK > &p_Area )
	{
		std::vector< TRACK >::const_iterator TrackItr = p_Area.begin( );

		unsigned int TrackNumber = 1UL;

		while( TrackItr != p_Area.end( ) )
		{
			std::string TypeString;

			if( ( *TrackItr ).Type == TRACK_TYPE_MODE1 )
			{
				TypeString = "MODE1";
			}
			else if( ( *TrackItr ).Type == TRACK_TYPE_CCDA )
			{
				TypeString = "CCDA";
			}
			else
			{
				TypeString = "UNKNOWN";
			}

			std::cout << "\tTrack " << TrackNumber << ": Start: " <<
				( *TrackItr ).StartAddress << " | Physical: " <<
				( *TrackItr ).PhysicalAddress << " | Size: " <<
				( *TrackItr ).Size << " | Type: " << TypeString << std::endl;
			++TrackNumber;
			++TrackItr;
		}
	}

	int CIMFile::WriteTracks( const std::vector< TRACK > &p_Area,
		const AREA p_AreaType, const unsigned int p_Index )
	{
		std::vector< TRACK >::const_iterator TrackItr = p_Area.begin( );
		unsigned int TrackIndex = p_Index + 1;
		LONG Offset = 0L;

		while( TrackItr != p_Area.end( ) )
		{
			DWORD BytesRead;
			size_t MemorySize = 2352 * ( *TrackItr ).Size;

			LPVOID pMemoryBlock = HeapAlloc( GetProcessHeap( ), 0,
				MemorySize );

			if( pMemoryBlock == NULL )
			{
				std::cout << "COULD NOT ALLOCATE MEMORY" << std::endl;
				return 1;
			}
			else
			{
				std::cout << "Allocated ";
				if( MemorySize < KiB )
				{
					std::cout << MemorySize << "B";
				}
				else if( MemorySize < MiB )
				{
					std::cout << MemorySize / KiB << "KiB";
				}
				else if( MemorySize < GiB )
				{
					std::cout << MemorySize / MiB << "MiB";
				}
				else
				{
					std::cout << MemorySize / GiB << "GiB";
				}
				std::cout << " RAM" <<
					std::endl;
			}

			if( p_Index == ( TrackIndex - 1 ) )
			{
				Offset = ( *TrackItr ).PhysicalAddress -
					( ( *TrackItr ).StartAddress * 2 );
			}

			LONG MoveTo = sizeof( DISC ) +
				( 2352 * ( ( *TrackItr ).StartAddress + Offset ) );

			switch( p_AreaType )
			{
			case AREA_SINGLE_DENSITY:
				{
					MoveTo += m_CIMHeader.SDADisc;
					break;
				}
			case AREA_HIGH_DENSITY:
				{
					MoveTo += m_CIMHeader.HDADisc;
					break;
				}
			}

			SetFilePointer( m_File, MoveTo, NULL, FILE_BEGIN );

			std::stringstream FileName;
			FileName << "track" << std::setw( 2 ) << std::setfill( '0' ) <<
				TrackIndex;
			
			if( ( *TrackItr ).Type == TRACK_TYPE_MODE1 )
			{
				FileName << ".bin";
			}
			else if( ( *TrackItr ).Type == TRACK_TYPE_CCDA )
			{
				FileName << ".raw";
			}

			std::cout << "Processing " << FileName.str( ) << " ... ";

			DWORD BytesWritten;

			ReadFile( m_File, pMemoryBlock, 2352 * ( *TrackItr ).Size,
				&BytesRead, NULL );

			HANDLE TrackFile = CreateFile( FileName.str( ).c_str( ),
				GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
				0 );
			
			WriteFile( TrackFile, pMemoryBlock, 2352 * ( *TrackItr ).Size,
				&BytesWritten, NULL );

			CloseHandle( TrackFile );

			HeapFree( GetProcessHeap( ), 0, pMemoryBlock );

			std::cout << "[OK]" << std::endl;

			++TrackIndex;
			++TrackItr;
		}

		return 0;
	}

	int CIMFile::WriteGDI( )
	{
		std::stringstream GDIContents;

		GDIContents << m_SDA.size( ) + m_HDA.size( ) << std::endl;

		std::vector< TRACK >::const_iterator TrackItr = m_SDA.begin( );
		unsigned int TrackIndex = 1;

		while( TrackItr != m_SDA.end( ) )
		{
			GDIContents << TrackIndex << " " << ( *TrackItr ).StartAddress <<
				" ";
			unsigned int Type;
			std::stringstream Name;
			Name << "track" << std::setw( 2 ) << std::setfill( '0' ) <<
				TrackIndex << ".";

			switch( ( *TrackItr ).Type )
			{
			case TRACK_TYPE_MODE1:
				{
					Name << "bin";
					Type = 4;

					break;
				}
			case TRACK_TYPE_CCDA:
				{
					Name << "raw";
					Type = 0;

					break;
				}
			}

			GDIContents << Type << " 2352 " << Name.str( ) << " 0" <<
				std::endl;

			++TrackItr;
			++TrackIndex;
		}

		TrackItr = m_HDA.begin( );
		TrackIndex = m_SDA.size( ) + 1;

		while( TrackItr != m_HDA.end( ) )
		{
			GDIContents << TrackIndex << " " << ( *TrackItr ).StartAddress <<
				" ";
			unsigned int Type;
			std::stringstream Name;
			Name << "track" << std::setw( 2 ) << std::setfill( '0' ) <<
				TrackIndex << ".";

			switch( ( *TrackItr ).Type )
			{
			case TRACK_TYPE_MODE1:
				{
					Name << "bin";
					Type = 4;

					break;
				}
			case TRACK_TYPE_CCDA:
				{
					Name << "raw";
					Type = 0;

					break;
				}
			}

			GDIContents << Type << " 2352 " << Name.str( ) << " 0" <<
				std::endl;

			++TrackItr;
			++TrackIndex;
		}

		HANDLE GDIFile = CreateFile( "game.gdi", GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

		DWORD BytesWritten;

		WriteFile( GDIFile, GDIContents.str( ).c_str( ),
			GDIContents.str( ).size( ), &BytesWritten, NULL );

		CloseHandle( GDIFile );

		return 0;
	}
}
