// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� OLEDLGVAX_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// OLEDLGVAX_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef OLEDLGVAX_EXPORTS
#define OLEDLGVAX_API __declspec(dllexport)
#else
#define OLEDLGVAX_API __declspec(dllimport)
#endif


int __cdecl PrintA(const char* fmt,...);

int __cdecl PrintW(const wchar_t* fmt,...);


VOID WINAPI Entry(HMODULE hModule);



#ifdef _UNICODE
#define Print PrintW
#else
#define Print PrintA
#endif