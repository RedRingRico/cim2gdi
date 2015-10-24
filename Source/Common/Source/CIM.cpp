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

		/*do
		{
			TRACK_ENTRY TrackEntry;

			ReadFile( m_File, &TrackEntry, sizeof( TrackEntry ),
				&BytesRead, NULL );

			++TrackCounter;

			unsigned int LSN = ( ( ( TrackEntry.Minutes & 0xF ) +
				( ( ( TrackEntry.Minutes & 0xF0 ) >> 4 ) * 10 ) * 60 * 75 ) +
				( ( TrackEntry.Seconds & 0x0F ) +
				( ( ( TrackEntry.Seconds & 0xF0 ) >> 4 ) * 10 ) ) * 75 ) +
				( TrackEntry.Frames & 0x0F ) +
				( ( ( TrackEntry.Frames & 0xF0 ) >> 4 ) * 10 );

			// The nn/00 track seems to be the beginning of the actual track
			// while nn/01 appears to indicate the offset to the data
			if( TrackEntry.Transform == 0 )
			{
				PreviousTrack = CurrentTrack;
				ZeroMemory( &CurrentTrack, sizeof( CurrentTrack ) );

				CurrentTrack.StartAddress = LSN;

				if( PreviousTrack.PhysicalAddress != 0 )
				{
					PreviousTrack.Size = CurrentTrack.StartAddress -
						PreviousTrack.PhysicalAddress;

					m_SDA.push_back( PreviousTrack );
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

					m_SDA.push_back( PreviousTrack );
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
		}while( Track );*/

		this->ExtractTracks( m_SDA );

		std::vector< TRACK >::const_iterator Itr = m_SDA.begin( );

		std::cout << "Processed tracks" << std::endl;

		unsigned int TrackNumber = 1UL;

		while( Itr != m_SDA.end( ) )
		{
			std::string TypeString;

			if( ( *Itr ).Type == TRACK_TYPE_MODE1 )
			{
				TypeString = "MODE1";
			}
			else if( ( *Itr ).Type == TRACK_TYPE_CCDA )
			{
				TypeString = "CCDA";
			}
			else
			{
				TypeString = "UNKNOWN";
			}

			std::cout << "\tTrack " << TrackNumber << ": Start: " <<
				( *Itr ).StartAddress << " | Physical: " <<
				( *Itr ).PhysicalAddress << " | Size: " <<
				( *Itr ).Size << " | Type: " << TypeString << std::endl;
			++TrackNumber;
			++Itr;
		}

		// Get the SDA Disc
		SetFilePointer( m_File, m_CIMHeader.SDADisc, NULL, FILE_BEGIN );

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

		// Copy the data from each track in the SDA to track[nm].[raw|bin]

		// Allocate enough memory to hold the entire SDA
		DWORD MemorySize = SDADisc.Size;
		
		LPVOID pMemoryBlock = HeapAlloc( GetProcessHeap( ), 0, MemorySize );

		if( pMemoryBlock == NULL )
		{
			std::cout << "COULD NOT ALLOCATE MEMORY" << std::endl;
			return 1;
		}
		else
		{
			std::cout << "Allocated " << MemorySize / MiB << "MiB RAM" <<
				std::endl;
		}

		std::vector< TRACK >::const_iterator TrackItr = m_SDA.begin( );
		unsigned int TrackCount = 1;

		while( TrackItr != m_SDA.end( ) )
		{
			SetFilePointer( m_File, m_CIMHeader.SDADisc + sizeof( SDADisc ) +
				( 2352 * ( *TrackItr ).PhysicalAddress ), NULL, FILE_BEGIN );

			std::stringstream FileName;
			FileName << "track" << std::setw( 2 ) << std::setfill( '0' ) <<
				TrackCount;
			
			if( ( *TrackItr ).Type == TRACK_TYPE_MODE1 )
			{
				FileName << ".bin";
			}
			else if( ( *TrackItr ).Type == TRACK_TYPE_CCDA )
			{
				FileName << ".raw";
			}

			std::cout << "Processing " << FileName.str( ) << std::endl;

			DWORD BytesWritten;

			ReadFile( m_File, pMemoryBlock, 2352 * ( *TrackItr ).Size,
				&BytesRead, NULL );

			HANDLE TrackFile = CreateFile( FileName.str( ).c_str( ),
				GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
				0 );
			
			WriteFile( TrackFile, pMemoryBlock, 2352 * ( *TrackItr ).Size,
				&BytesWritten, NULL );

			CloseHandle( TrackFile );

			++TrackCount;
			++TrackItr;
		}

		HeapFree( GetProcessHeap( ), 0, pMemoryBlock );

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

			unsigned int LSN = ( ( ( TrackEntry.Minutes & 0xF ) +
				( ( ( TrackEntry.Minutes & 0xF0 ) >> 4 ) * 10 ) * 60 * 75 ) +
				( ( TrackEntry.Seconds & 0x0F ) +
				( ( ( TrackEntry.Seconds & 0xF0 ) >> 4 ) * 10 ) ) * 75 ) +
				( TrackEntry.Frames & 0x0F ) +
				( ( ( TrackEntry.Frames & 0xF0 ) >> 4 ) * 10 );

			// The nn/00 track seems to be the beginning of the actual track
			// while nn/01 appears to indicate the offset to the data
			if( TrackEntry.Transform == 0 )
			{
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
}
