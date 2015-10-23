#ifndef __CIM2GDI_CIM_HPP__
#define __CIM2DGI_CIM_HPP__

#include <windows.h>
#include <string>

namespace cim2gdi
{
	enum AREA
	{
		AREA_SINGLE_DENSITY = 0x00000001,
		AREA_HIGH_DENSITY	= 0x00000002
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

	struct SDA_TOC
	{
		char			ID[ 4 ];
		unsigned int	SDADisc;
		char			Padding[ 8 ];
	};

	struct SDA_DISC
	{
		char			ID[ 4 ];
		unsigned int	Size;
		char			Padding[ 8 ];
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

	private:
		HANDLE	m_File;
	};
}

//NOTES
// 0x0562A0 - 150 physical sectors in (0)
// 0x1BAC10 - 771 physical sectors in (921) (02/00)
// 0x214790 - Start of audio track

// 0x101DB0 - Last sector for 00:06:00
// 0x1026E0 - End of sector
// 0x15C26C - Beginning of the audio
// 0x05940C - Distance to audio
// 0x233E2C - End of audio 

#endif // __CIM2GDI_CIM_HPP__
