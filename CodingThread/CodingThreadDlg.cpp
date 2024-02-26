#include <string>
#include "CodingThreadDlg.h"
CodingThreadDlg* CodingThreadDlg::ptr = NULL;
HANDLE hMutex;

CodingThreadDlg::CodingThreadDlg(void)
{
	ptr = this;
}

CodingThreadDlg::~CodingThreadDlg(void)
{
	
}

void CodingThreadDlg::Cls_OnClose(HWND hwnd)
{
	ReleaseMutex(hMutex); 
	EndDialog(hwnd, 0);

}
void CodingThreadDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BUTTON1) {
		ReleaseMutex(hMutex); 
		EndDialog(hwnd, 0);
	}
}

DWORD WINAPI Coding_Thread(LPVOID lp)
{
	CodingThreadDlg *ptr = (CodingThreadDlg *)lp;
	char buf[100];
	ifstream in(TEXT("music.txt"),ios::in);
	if(!in)
	{
		MessageBox(ptr->hDialog, TEXT("Ошибка открытия файла!"), TEXT("Мьютекс"), MB_OK | MB_ICONINFORMATION);
		return 1;
	}

	hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("{B8A2C367-10FE-494d-A869-841B2AF972E0}"));

	DWORD dwAnswer = WaitForSingleObject(hMutex, INFINITE);

		ofstream out("coding.txt", ios::out | ios::trunc);
		const int KEY = 100;
		while(!in.eof())
		{
			in.getline(buf,100);
			out << buf << endl;
		}
		out.close();
		in.close();

		MessageBox(ptr->hDialog, TEXT("Запись данных в файл coding.txt завершена!"), TEXT("Мьютекс"), MB_OK | MB_ICONINFORMATION);

		ReleaseMutex(hMutex);

	return 0;
}

DWORD WINAPI EditControl_Thread(LPVOID lp) {
	CodingThreadDlg* ptr = (CodingThreadDlg*)lp;
	HWND hEdit = GetDlgItem(ptr->hDialog, IDC_EDIT1);

	if (!hEdit) {
		MessageBox(ptr->hDialog, TEXT("Ошибка получения доступа к элементу управления!"), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
		ReleaseMutex(hMutex);
		return 1;
	}

	ifstream in(TEXT("coding.txt"), ios::in);
	if (!in) {
		MessageBox(ptr->hDialog, TEXT("Ошибка открытия файла!"), TEXT("Мьютекс"), MB_OK | MB_ICONINFORMATION);
		ReleaseMutex(hMutex);
		return 1;
	}

	hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("{B8A2C367-10FE-494d-A869-841B2AF972E0}"));
	DWORD dwAnswer = WaitForSingleObject(hMutex, INFINITE);

	string text;
	string line;
	while (getline(in, line)) {
		text += line + "\r\n"; 
	}

	int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
	wchar_t* wbuf = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wbuf, len);

	SetWindowText(hEdit, wbuf);

	delete[] wbuf;
	ReleaseMutex(hMutex);
	return 0;
}






BOOL CodingThreadDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	hDialog = hwnd;
	hMutex = CreateMutex(NULL, FALSE, TEXT("{B8A2C367-10FE-494d-A869-841B2AF972E0}")); // Создание мьютекса
	if (!hMutex) {
		MessageBox(hwnd, TEXT("Ошибка создания мьютекса!"), TEXT("Мьютекс"), MB_OK | MB_ICONERROR);
		EndDialog(hwnd, 0);
		return FALSE;
	}

	HANDLE hThread = CreateThread(NULL, 0, Coding_Thread, this, 0, NULL);
	if (!hThread) {
		MessageBox(hwnd, TEXT("Ошибка создания потока Coding_Thread!"), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
		CloseHandle(hMutex);
		EndDialog(hwnd, 0);
		return FALSE;
	}
	CloseHandle(hThread);

	hThread = CreateThread(NULL, 0, EditControl_Thread, this, 0, NULL);
	if (!hThread) {
		MessageBox(hwnd, TEXT("Ошибка создания потока EditControl_Thread!"), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
		CloseHandle(hMutex); 
		EndDialog(hwnd, 0);
		return FALSE;
	}
	CloseHandle(hThread);

	return TRUE;
}



BOOL CALLBACK CodingThreadDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}