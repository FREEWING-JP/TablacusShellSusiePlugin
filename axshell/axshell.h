#include "resource.h"
#include <windows.h>

#include <Shlobj.h>
#include <Shellapi.h>
#include <shobjidl.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <propkey.h>
#pragma comment(lib, "shlwapi.lib")

#define SUSIE_EXT	"_SF"
#define FILE_HEADER	"_SF"
#define FILE_HEADER_SIZE 3

#define	SIZE_BUFF 32768
#define	CACHE_ITEMS 65536
#define MAX_CSIDL				256
#define E_CANCELLED         HRESULT_FROM_WIN32(ERROR_CANCELLED)
#define E_FILE_NOT_FOUND    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)
#define E_PATH_NOT_FOUND    HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)
#define E_NOT_READY         HRESULT_FROM_WIN32(ERROR_NOT_READY)
#define E_BAD_NETPATH       HRESULT_FROM_WIN32(ERROR_BAD_NETPATH)
#define E_INVALID_PASSWORD  HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD)
// Susie Plug-in �֘A�̒�` ---------------------------------------------------
/*-------------------------------------------------------------------------*/
/* �G���[�R�[�h */
/*-------------------------------------------------------------------------*/
#define SPI_NO_FUNCTION			-1	/* ���̋@�\�̓C���v�������g����Ă��Ȃ� */
#define SPI_ALL_RIGHT			0	/* ����I�� */
#define SPI_ABORT				1	/* �R�[���o�b�N�֐�����0��Ԃ����̂œW�J�𒆎~���� */
#define SPI_NOT_SUPPORT			2	/* ���m�̃t�H�[�}�b�g */
#define SPI_OUT_OF_ORDER		3	/* �f�[�^�����Ă��� */
#define SPI_NO_MEMORY			4	/* �������[���m�ۏo���Ȃ� */
#define SPI_MEMORY_ERROR		5	/* �������[�G���[ */
#define SPI_FILE_READ_ERROR		6	/* �t�@�C�����[�h�G���[ */
#define	SPI_WINDOW_ERROR		7	/* �����J���Ȃ� (����J�̃G���[�R�[�h) */
#define SPI_OTHER_ERROR			8	/* �����G���[ */
#define	SPI_FILE_WRITE_ERROR	9	/* �������݃G���[ (����J�̃G���[�R�[�h) */
#define	SPI_END_OF_FILE			10	/* �t�@�C���I�[ (����J�̃G���[�R�[�h) */

//-------------------------------------- DLL �萔
typedef ULONG_PTR susie_time_t;
//-------------------------------------- DLL �\����
#pragma pack(push,1)

typedef struct {
	unsigned char  method[8];	// ���k�@�̎��
	ULONG_PTR      position;	// �t�@�C����ł̈ʒu
	ULONG_PTR      compsize;	// ���k���ꂽ�T�C�Y
	ULONG_PTR      filesize;	// ���̃t�@�C���T�C�Y
	susie_time_t   timestamp;	// �t�@�C���̍X�V����
	char           path[200];	// ���΃p�X
	char           filename[200];	// �t�@�C����
	unsigned long  crc;	// CRC
	#ifdef _WIN64
	   // 64bit�ł̍\���̃T�C�Y��444bytes�ł����A���ۂ̃T�C�Y��
	   // �A���C�������g�ɂ��448bytes�ɂȂ�܂��B���ɂ��dummy���K�v�ł��B
	   char        dummy[4];
	#endif
} SUSIE_FINFO;
#pragma pack(pop)

typedef struct {
	unsigned char	method[8];		// ���k�@�̎��
	ULONG_PTR		position;		// �t�@�C����ł̈ʒu
	ULONG_PTR		compsize;		// ���k���ꂽ�T�C�Y
	ULONG_PTR		filesize;		// ���̃t�@�C���T�C�Y
	susie_time_t	timestamp;		// �t�@�C���̍X�V����
	WCHAR			path[200];		// ���΃p�X
	WCHAR			filename[200];	// �t�@�C���l�[��
	unsigned long	crc;			// CRC
}SUSIE_FINFOTW;

// �R�[���o�b�N
typedef int (__stdcall *SUSIE_PROGRESS)(int nNum,int nDenom,LONG_PTR lData);

//XP or higher.
typedef HRESULT (WINAPI* LPFNSHParseDisplayName)(LPCWSTR pszName, IBindCtx *pbc, PIDLIST_ABSOLUTE *ppidl, SFGAOF sfgaoIn, SFGAOF *psfgaoOut);
