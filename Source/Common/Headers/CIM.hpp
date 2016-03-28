#ifndef __CIM2GDI_CIM_HPP__
#define __CIM2DGI_CIM_HPP__

#include <windows.h>
#include <string>
#include <vector>

namespace cim2gdi
{
	enum AREA
	{
		AREA_SINGLE_DENSITY = 0x00000001,
		AREA_HIGH_DENSITY	= 0x00000002
	};

	enum TRACK_TYPE
	{
		TRACK_TYPE_MODE1	= 1,
		TRACK_TYPE_CCDA		= 2
	};

#pragma pack( push, 1 )
	// Each chunk in the file has the format of a 4CC and either an offset to
	// another chunk or the size of the chunk
	struct CIM_HEADER
	{
		char			ID[ 4 ];
		unsigned int	FileSize;
		// Presumably, this is a leftover from the Saturn?
		char			ImageType[ 4 ];
		// The area flags indicate whether it's a full image (1) or just enough
		// data without the padding (2)
		unsigned int	SDAFlags;
		unsigned int	HDAFlags;
		// Single density area
		// Offset to the single density TOC (from the start of the file)
		unsigned int	SDATOC;
		// Offset to the single density disc
		unsigned int	SDADisc;
		// High density area
		unsigned int	HDATOC;
		unsigned int	HDADisc;
		// 16-byte alignment
		// It appears there is a series of flags (either three or four)
		// after the HDADisc, so there are 9 or 8 bytes of padding in reality
		char			Padding[ 12 ];
	};
    
	// Nothing interesting in the TOC, immediately folling it is the track
	// information
	struct TOC
	{
		char			ID[ 4 ];
		// Offset to the disc folling this chunk
		unsigned int	DiscOffset;
		char			Padding[ 8 ];
	};

	// Just contains the size of the disc image
	struct DISC
	{
		char			ID[ 4 ];
		unsigned int	Size;
		char			Padding[ 8 ];
	};

	// Each entry needs to be decoded to acquire the track start/end/padding
	struct TRACK_ENTRY
	{
		// Data 0x41/Audio 0x01
		unsigned char	Control;
		// Canonical track number
		unsigned char	TrackNumber;
		// IDX 0 - pregap, 1 - track
        unsigned char	Index;
		// MODE1 0x01/CCDA 0x02
		unsigned char	Form;
		// The following are in BCD (hours may be padding)
		unsigned char	Hours;
		unsigned char	Minutes;
		unsigned char	Seconds;
		unsigned char	Frames;
	};

	// Not in the CIM, used for managing tracks for ease of processing
	struct TRACK
	{
		unsigned int	PhysicalAddress;
		unsigned int	StartAddress;
		unsigned int	TNO;
		unsigned int	IDX;
		unsigned int	Size;
		TRACK_TYPE		Type;

		unsigned int	doffset;
		unsigned int	dsize;
		unsigned int	dskip;
	};
#pragma pack( pop )

	class CIMFile
	{
	public:
		CIMFile( );
		~CIMFile( );

		int Open( const std::string &p_FileName );
		void Close( );

		int Parse( );

		int GetSDATrackCount( ) const;
		int GetHDATrackCount( ) const;

		int GetTrackAt( bool p_SDA, int Track, TRACK &p_Track,
			const AREA p_AreaType ) const;

	private:
		int ParseHeader( );
		int ExtractTracks( std::vector< TRACK > &p_Area );
		void PrintTrackInformation( const std::vector< TRACK > &p_Area );
		int WriteTracks( const std::vector< TRACK > &p_Area,
			const AREA p_AreaType, const unsigned int p_Index = 0 );
		int WriteGDI( );

		HANDLE	m_File;

		CIM_HEADER	m_CIMHeader;

		std::vector< TRACK >	m_SDA;
		std::vector< TRACK >	m_HDA;
	};
}

#endif // __CIM2GDI_CIM_HPP__
