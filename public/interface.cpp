#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "interface.h"

#if !defined ( _WIN32 )
// Linux doesn't have this function so this emulates its functionality
//
//
void *GetModuleHandle(const char *name)
{
        void *handle;


        if( name == NULL )
        {
                // hmm, how can this be handled under linux....
                // is it even needed?
                return NULL;
        }

        if( (handle=dlopen(name, RTLD_NOW))==NULL)
        {   
		//printf("Error:%s\n",dlerror());
                // couldn't open this file
                return NULL;
        }

        // read "man dlopen" for details
        // in short dlopen() inc a ref count
        // so dec the ref count by performing the close
        dlclose(handle);
       return handle;
}
#endif

// ------------------------------------------------------------------------------------ //
// InterfaceReg.
// ------------------------------------------------------------------------------------ //
InterfaceReg *InterfaceReg::s_pInterfaceRegs = NULL;


InterfaceReg::InterfaceReg( InstantiateInterfaceFn fn, const char *pName ) :
	m_pName(pName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}



// ------------------------------------------------------------------------------------ //
// CreateInterface.
// ------------------------------------------------------------------------------------ //
EXPORT_FUNCTION IBaseInterface *CreateInterface( const char *pName, int *pReturnCode )
{
	InterfaceReg *pCur;
	
	for(pCur=InterfaceReg::s_pInterfaceRegs; pCur; pCur=pCur->m_pNext)
	{
		if(strcmp(pCur->m_pName, pName) == 0)
		{
			if ( pReturnCode )
			{
				*pReturnCode = IFACE_OK;
			}
			return pCur->m_CreateFn();
		}
	}
	
	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;	
}

#ifdef LINUX
static IBaseInterface *CreateInterfaceLocal( const char *pName, int *pReturnCode )
{
	InterfaceReg *pCur;
	
	for(pCur=InterfaceReg::s_pInterfaceRegs; pCur; pCur=pCur->m_pNext)
	{
		if(strcmp(pCur->m_pName, pName) == 0)
		{
			if ( pReturnCode )
			{
				*pReturnCode = IFACE_OK;
			}
			return pCur->m_CreateFn();
		}
	}
	
	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;	
}
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : pModuleName - module name
//			*pName - proc name
//-----------------------------------------------------------------------------
//static hlds_run wants to use this function 
static void *Sys_GetProcAddress( const char *pModuleName, const char *pName )
{
	return Sys_GetProcAddress( GetModuleHandle(pModuleName), pName );
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : pModuleName - module name
//			*pName - proc name
//-----------------------------------------------------------------------------
// hlds_run wants to use this function 
void *Sys_GetProcAddress( void *pModuleHandle, const char *pName )
{
#if defined ( _WIN32 )
	return GetProcAddress( ( HMODULE ) pModuleHandle, pName );
#else
	return dlsym( pModuleHandle, pName );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Loads a DLL/component from disk and returns a handle to it
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
CSysModule	*Sys_LoadModule( const char *pModuleName )
{
#if defined ( _WIN32 )
	HMODULE hDLL = LoadLibrary( pModuleName );
#else
	void* hDLL  = NULL;
	char szAbsoluteModuleName[1024];
	szAbsoluteModuleName[0] = 0;
	if ( pModuleName[0] != '/' )
	{
		char szCwd[1024];
		char szAbsoluteModuleName[1024];

		getcwd( szCwd, sizeof( szCwd ) );
		if ( szCwd[ strlen( szCwd ) - 1 ] == '/' )
			szCwd[ strlen( szCwd ) - 1 ] = 0;

		_snprintf( szAbsoluteModuleName, sizeof(szAbsoluteModuleName), "%s/%s", szCwd, pModuleName );

		hDLL = dlopen( szAbsoluteModuleName, RTLD_NOW );
	}
	else
	{
		_snprintf( szAbsoluteModuleName, sizeof(szAbsoluteModuleName), "%s", pModuleName );
		 hDLL = dlopen( pModuleName, RTLD_NOW );
	}
#endif

	if( !hDLL )
	{
		char str[512];
#if defined ( _WIN32 )
		snprintf( str, sizeof(str), "%s.dll", pModuleName );
		hDLL = LoadLibrary( str );
#elif defined(OSX)
		printf("Error:%s\n",dlerror());
		snprintf( str, sizeof(str), "%s.dylib", szAbsoluteModuleName );
		hDLL = dlopen(str, RTLD_NOW);		
#else
		printf("Error:%s\n",dlerror());
		snprintf( str, sizeof(str), "%s.so", szAbsoluteModuleName );
		hDLL = dlopen(str, RTLD_NOW);
#endif
	}

	return reinterpret_cast<CSysModule *>(hDLL);
}

//-----------------------------------------------------------------------------
// Purpose: Unloads a DLL/component from
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
void Sys_UnloadModule( CSysModule *pModule )
{
	if ( !pModule )
		return;

#if defined ( _WIN32 )
	FreeLibrary( reinterpret_cast<HMODULE>( pModule ) );
#else
	dlclose( reinterpret_cast<void*>( pModule ) );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : module - module handle from Sys_LoadModule() 
//			*pName - proc name
// Output : factory for this module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory( CSysModule *pModule )
{
	if ( !pModule )
		return nullptr;

	void* pFunction = Sys_GetProcAddress( pModule, CREATEINTERFACE_PROCNAME );

#ifdef WIN32
	return reinterpret_cast<CreateInterfaceFn>( pFunction );
#else
	// Linux gives this error:
	//../public/interface.cpp: In function `IBaseInterface *(*Sys_GetFactory 
	//(CSysModule *)) (const char *, int *)':
	//../public/interface.cpp:154: ISO C++ forbids casting between 
	//pointer-to-function and pointer-to-object
	//
	// so lets get around it :)
	return ( CreateInterfaceFn ) pFunction;
#endif
}



//-----------------------------------------------------------------------------
// Purpose: returns the instance of this module
// Output : interface_instance_t
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactoryThis( void )
{
#ifdef LINUX
	return CreateInterfaceLocal;
#else
	return CreateInterface;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: returns the instance of the named module
// Input  : *pModuleName - name of the module
// Output : interface_instance_t - instance of that module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory( const char *pModuleName )
{
#if defined ( _WIN32 )
	return static_cast<CreateInterfaceFn>( Sys_GetProcAddress( pModuleName, CREATEINTERFACE_PROCNAME ) );
#else
// Linux gives this error:
//../public/interface.cpp: In function `IBaseInterface *(*Sys_GetFactory 
//(const char *)) (const char *, int *)':
//../public/interface.cpp:186: invalid static_cast from type `void *' to 
//type `IBaseInterface *(*) (const char *, int *)'
//
// so lets use the old style cast.
	return (CreateInterfaceFn)( Sys_GetProcAddress( pModuleName, CREATEINTERFACE_PROCNAME ) );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: finds a particular interface in the factory set
//-----------------------------------------------------------------------------
void *InitializeInterface( char const *interfaceName, CreateInterfaceFn *factoryList, int numFactories )
{
	void *retval;

	for( int i = 0; i < numFactories; i++ )
	{
		CreateInterfaceFn factory = factoryList[ i ];
		if( !factory )
			continue;

		retval = factory( interfaceName, NULL );
		if( retval )
			return retval;
	}

	// No provider for requested interface!!!
	// Assert( !"No provider for requested interface!!!" );

	return NULL;
}

