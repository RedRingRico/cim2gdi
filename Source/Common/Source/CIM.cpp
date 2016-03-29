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

		SDATOC.Size = SwapUInt32( SDATOC.Size );

		std::cout << "Single Density Area TOC Information" << std::endl;

		std::cout << "\tID:   " << IDString << std::endl;

		std::cout << std::setfill( '0' ) << std::hex;
		std::cout << "\tSize: 0x" << std::setw( 8 ) <<
			SDATOC.Size << std::endl;
		std::cout << std::setfill( ' ' ) << std::dec;

		this->ExtractTracks(m_SDA, SDATOC.Size / 8);

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

		HDATOC.Size = SwapUInt32( HDATOC.Size );

		std::cout << "High Density Area TOC Information" << std::endl;

		std::cout << "\tID:   " << IDString << std::endl;

		std::cout << std::setfill( '0' ) << std::hex;
		std::cout << "\tSize: 0x" << std::setw( 8 ) <<
			HDATOC.Size << std::endl;
		std::cout << std::setfill( ' ' ) << std::dec;

		this->ExtractTracks(m_HDA, HDATOC.Size / 8);

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

	int CIMFile::ExtractTracks(std::vector< TRACK > &p_Area, unsigned int entrynum)
	{
		DWORD BytesRead;

		TRACK CurrentTrack, PreviousTrack;
		ZeroMemory( &CurrentTrack, sizeof( CurrentTrack ) );
		ZeroMemory( &PreviousTrack, sizeof( PreviousTrack ) );

		unsigned int cur_offset = 0;

		std::cout << "\tTrack Information" << std::endl;

		while (entrynum--)
		{
			TRACK_ENTRY TrackEntry;

			ReadFile( m_File, &TrackEntry, sizeof( TrackEntry ),
				&BytesRead, NULL );

			unsigned int LSN =
				( ( ( TrackEntry.Minutes & 0xF ) +
					( ( ( TrackEntry.Minutes & 0xF0 ) >> 4 ) * 10 ) )
						* 60 * 75 ) +
				( ( ( TrackEntry.Seconds & 0x0F ) +
					( ( ( TrackEntry.Seconds & 0xF0 ) >> 4 ) * 10 ) ) * 75 ) +
				( TrackEntry.Frames & 0x0F ) +
					( ( ( TrackEntry.Frames & 0xF0 ) >> 4 ) * 10 );

			PreviousTrack = CurrentTrack;

			ZeroMemory(&CurrentTrack, sizeof(CurrentTrack));
			CurrentTrack.StartAddress = LSN - 150;
			CurrentTrack.PhysicalAddress = LSN;
			CurrentTrack.TNO = (TrackEntry.TrackNumber & 0x0F) + ((TrackEntry.TrackNumber & 0xF0) >> 4) * 10;
			CurrentTrack.IDX = TrackEntry.Index;

			PreviousTrack.Size = CurrentTrack.PhysicalAddress - PreviousTrack.PhysicalAddress;
			// now we have prev track size in sectors, calculate it's data lenght and get current track data offset
			if (PreviousTrack.TNO != 0)
				cur_offset += PreviousTrack.Size * ((PreviousTrack.Type == TRACK_TYPE_MODE1) ? 2352 : 2450);
			CurrentTrack.doffset = cur_offset;

			if (PreviousTrack.TNO > 0 && PreviousTrack.TNO <= 99 && PreviousTrack.IDX != 0)
				p_Area.push_back(PreviousTrack);

			// for some weird reason in GDI format, if audio track followed by data track, that next track's audio-type pregap must be part of previos track
			// detect this case and append pregap to previous track
			if (PreviousTrack.TNO == CurrentTrack.TNO && PreviousTrack.IDX == 0 && CurrentTrack.IDX == 0 && PreviousTrack.Type != CurrentTrack.Type)
				p_Area.back().Size += PreviousTrack.Size;

			std::string TrackType = TrackEntry.Control == 0x41 ?
				"DATA" : "AUDIO";
			std::string TrackForm;
			
			if( TrackEntry.Form == 0x02 )
			{
				TrackForm = "MODE1";
				CurrentTrack.Type = TRACK_TYPE_MODE1;
				CurrentTrack.dsize = 2048;			// Extract data tracks as 2048, RAW 2352 is useless because CIM Utility doesn't generate EDC
				CurrentTrack.dskip = 304;
				CurrentTrack.doffset += 16;			// skip header
			}
			if( TrackEntry.Form == 0x00 )
			{
				TrackForm = "CCDA";
				CurrentTrack.Type = TRACK_TYPE_CCDA;
				CurrentTrack.dsize = 2352;
				CurrentTrack.dskip = 98;
			}

			std::cout << "\t\t" << TrackType << " ";

			if( TrackEntry.TrackNumber == 0x00 )
			{
				std::cout << "LeadIn";
			}
			else if( TrackEntry.TrackNumber == 0xAA )
			{
				std::cout << "LeadOut";
			}
			else
			{
				std::cout <<+TrackEntry.TrackNumber << "/" <<
					+TrackEntry.Index;
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

		}

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

		while( TrackItr != p_Area.end( ) )
		{
			DWORD BytesRead;
			size_t MemorySize = ( *TrackItr ).dsize;

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

			LONG MoveTo = sizeof(DISC);

			switch( p_AreaType )
			{
			case AREA_SINGLE_DENSITY:
				MoveTo += m_CIMHeader.SDADisc;
				break;
			case AREA_HIGH_DENSITY:
				MoveTo += m_CIMHeader.HDADisc;
				break;
			}

			MoveTo += (*TrackItr).doffset;

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

			HANDLE TrackFile = CreateFile(FileName.str().c_str(),
				GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
				0);

			for (unsigned int i = 0; i < (*TrackItr).Size; i++) {
				ReadFile(m_File, pMemoryBlock, (*TrackItr).dsize,
					&BytesRead, NULL);

				WriteFile(TrackFile, pMemoryBlock, (*TrackItr).dsize,
					&BytesWritten, NULL);

				SetFilePointer(m_File, (*TrackItr).dskip, NULL, FILE_CURRENT);
			}

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

			GDIContents << Type << " " << (*TrackItr).dsize << " " << Name.str( ) << " 0" <<
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

			GDIContents << Type << " " << (*TrackItr).dsize << " " << Name.str() << " 0" <<
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
