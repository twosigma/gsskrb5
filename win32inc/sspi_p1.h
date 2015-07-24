#define SECPKG_ATTR_NATIVE_NAMES    13
typedef struct _SecPkgContext_NativeNamesW
{
    SEC_WCHAR SEC_FAR * sClientName;
    SEC_WCHAR SEC_FAR * sServerName;
} SecPkgContext_NativeNamesW, SEC_FAR * PSecPkgContext_NativeNamesW;

typedef struct _SecPkgContext_NativeNamesA
{
    SEC_CHAR SEC_FAR * sClientName;
    SEC_CHAR SEC_FAR * sServerName;
} SecPkgContext_NativeNamesA, SEC_FAR * PSecPkgContext_NativeNamesA;


#ifdef UNICODE
#  define SecPkgContext_NativeNames SecPkgContext_NativeNamesW
#  define PSecPkgContext_NativeNames PSecPkgContext_NativeNamesW
#else
#  define SecPkgContext_NativeNames SecPkgContext_NativeNamesA
#  define PSecPkgContext_NativeNames PSecPkgContext_NativeNamesA
#endif // !UNICODE

