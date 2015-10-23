#include <Utility.hpp>

namespace cim2gdi
{
	unsigned int SwapUInt32( const unsigned int p_UInt )
	{
		return ( ( p_UInt & 0x000000FF ) << 24 ) |
			( ( p_UInt & 0x0000FF00 ) << 8 ) |
			( ( p_UInt & 0x00FF0000 ) >> 8 ) |
			( ( p_UInt & 0xFF000000 ) >> 24 );
	}
}
