#include <iostream>
#include <CIM.hpp>

int main( int p_Argc, char **p_ppArgv )
{
	std::cout << "cim2gdi" << std::endl;

	if( p_Argc < 2 )
	{
		return 1;
	}

	cim2gdi::CIMFile CIM;

	if( CIM.Open( p_ppArgv[ 1 ] ) != 0 )
	{
		return 1;
	}

	if( CIM.Parse( ) != 0 )
	{
		return 1;
	}

	CIM.Close( );

	return 0;
}
