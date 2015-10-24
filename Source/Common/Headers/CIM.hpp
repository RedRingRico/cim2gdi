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
	struct CIM_HEADER
	{
		char			ID[ 4 ];
		unsigned int	FileSize;
		char			ImageType[ 4 ];
		unsigned int	SDAFlags;
		unsigned int	HDAFlags;
		unsigned int	SDATOC;
		unsigned int	SDADisc;
		unsigned int	HDATOC;
		unsigned int	HDADisc;
		char			Padding[ 12 ];
	};

	struct TOC
	{
		char			ID[ 4 ];
		unsigned int	DiscOffset;
		char			Padding[ 8 ];
	};

	struct DISC
	{
		char			ID[ 4 ];
		unsigned int	Size;
		char			Padding[ 8 ];
	};

	struct TRACK
	{
		unsigned int	PhysicalAddress;
		unsigned int	StartAddress;
		unsigned int	Size;
		TRACK_TYPE		Type;
	};

	struct TRACK_ENTRY
	{
		unsigned char	Control;
		unsigned char	TrackNumber;
        unsigned char	Transform;
		unsigned char	Form;
		// BCD
		unsigned char	Hours;
		unsigned char	Minutes;
		unsigned char	Seconds;
		unsigned char	Frames;
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
			const AREA p_AreaType, const int p_Index = 0 );
		int WriteGDI( );

		HANDLE	m_File;

		CIM_HEADER	m_CIMHeader;

		std::vector< TRACK >	m_SDA;
		std::vector< TRACK >	m_HDA;
	};
}

#endif // __CIM2GDI_CIM_HPP__
