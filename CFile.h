#ifndef FILE_H
#define FILE_H

#define FILE_PRINTF_BUFFER_SIZE 255
#define FILE_SCANF_BUFFER_SIZE 1024
#define MAX_FILENAME_PATH 255

#include <tchar.h>

#ifdef _UNICODE
	#define SPRINTF vswprintf
	#define SSCANF swscanf
	#define isUnicode 1
#else
	#define SPRINTF vsprintf
	#define isUnicode 0
#endif

enum FileAccess{FileWrite,FileRead,FileReadWrite};

class CFile
{
	OPENFILENAME	browseDialogStruct;
	HANDLE	file;	//uchwyt do pliku
	TCHAR	buffer[FILE_PRINTF_BUFFER_SIZE];	//bufor danych pliku wykorzystywany przez funkcje typu *printf
	char	buffer_ansi[FILE_PRINTF_BUFFER_SIZE]; //tymczasowy bufor zapisu wykorzystywany do konwersji z UNICODE do ANSI 
	TCHAR	read_buffer[FILE_SCANF_BUFFER_SIZE];	//bufor odczytywanych danych
	char	read_buffer_ansi[FILE_SCANF_BUFFER_SIZE]; //tymczasowy bufor odczytu wykorzystywany do konwersji z UNICODE do ANSI 
	TCHAR	fileName[MAX_FILENAME_PATH];	//sciezka do pliku
	DWORD	dataSizeInReadBuffer;
public:
	CFile(){fileName[0]=0;file=0;}
	CFile(TCHAR *fileName, FileAccess desiredAccess){OpenFile(fileName,desiredAccess);}

	/// <summary>Konwertuje ciag znakow zapisany w tablicy char[] do tablicy wchar_t[]</summary>
	///	<param name="ansi">wskaznik do istniejacego ciagu znakow w tablicy typu char</param>
	///	<param name="unicode">wskaznik do docelowej tablicy typu wchar_t</param>
	///	<param name="ansiSize">maksymalna liczba znakow do przekopiowania (bez znaku NULL); moze zostac pominiety, wowczas elementy kopiowane sa dopoki nie pojawi sie znak NULL</param>
	/// <returns>liczba przekopiowanych znakow</returns>
	static unsigned int char2wchar(char *ansi, wchar_t *unicode, unsigned int ansiSize=0xffffffff);

	/// <summary>Konwertuje ciag znakow zapisany w tablicy wchar_t[] do tablicy char[]</summary>
	///	<param name="unicode">wskaznik do tablicy typu wchar_t zawierajacej ciag znakow</param>
	///	<param name="ansi">wskaznik do docelowej tablicy typu char</param>
	///	<param name="ansiSize">maksymalna liczba znakow do przekopiowania (bez znaku NULL); moze zostac pominiety, wowczas elementy kopiowane sa dopoki nie pojawi sie znak NULL</param>
	/// <returns>liczba przekopiowanych znakow</returns>
	static unsigned int wchar2char(wchar_t *unicode, char *ansi, unsigned int unicodeSize=0xffffffff);

	/// <summary>Otwiera okno dialogowe wyboru sciezki otwieranego pliku. Sciezka pliku zapisywana jest w polu fileName.</summary>
	///	<param name="ext">domyœlne rozszerzeniu pliku, domyœlnie NULL - brak rozszerzenia</param>
	///	<param name="desc">opis domyœlnego rozszerzenia pliku, domyœlnie NULL - brak opisu</param>
	/// <returns>zwraca wartosc rozna od zera gdy poprawnie wybrano plik; zwraca 0 gdy otwieranie jest anulowane lub zakonczone niepowodzeniem</returns>
	int OpenFilename(std::wstring ext = std::wstring(), std::wstring desc = std::wstring());

	/// <summary>Otwiera okno dialogowe wyboru sciezki zapisywanego pliku. Sciezka pliku zapisywana jest w polu fileName.</summary>
	///	<param name="ext">domyœlne rozszerzeniu pliku, domyœlnie NULL - brak rozszerzenia</param>
	///	<param name="desc">opis domyœlnego rozszerzenia pliku, domyœlnie NULL - brak opisu</param>
	/// <returns>zwraca wartosc rozna od zera gdy poprawnie wybrano plik; zwraca 0 gdy otwieranie jest anulowane lub zakonczone niepowodzeniem</returns>
	int SaveFilename(std::wstring ext = std::wstring(), std::wstring desc = std::wstring());

	/// <summary>Otwiera plik</summary>
	///	<param name="fileName">Sciezka pliku</param>
	///	<param name="desiredAccess">Porzadany dostep do pliku. Mozliwe opcje: FileWrite,FileRead,FileReadWrite</param>
	/// <returns>Zwraca uchwyt do pliku (NIE DZIALA Z FUNKCJAMI fprintf itp.). W przypadku niepowodzenia zwraca 0.</returns>
	int OpenFile(TCHAR *fileName, FileAccess desiredAccess);

	/// <summary>Otwiera okno dialogowe wyboru sciezki otwieranego pliku i otwiera plik do odczytu</summary>
	///	<param name="ext">domyœlne rozszerzeniu pliku, domyœlnie NULL - brak rozszerzenia</param>
	///	<param name="desc">opis domyœlnego rozszerzenia pliku, domyœlnie NULL - brak opisu</param>
	/// <returns>Zwraca uchwyt do pliku (NIE DZIALA Z FUNKCJAMI fprintf itp.). W przypadku niepowodzenia lub anulowania zwraca 0.</returns>
	int SelectAndOpen(std::wstring ext = std::wstring(), std::wstring desc = std::wstring());

	/// <summary>Otwiera okno dialogowe wyboru sciezki zapisywanego pliku i otwiera plik do zapisu</summary>
	///	<param name="ext">domyœlne rozszerzeniu pliku, domyœlnie NULL - brak rozszerzenia</param>
	///	<param name="desc">opis domyœlnego rozszerzenia pliku, domyœlnie NULL - brak opisu</param>
	/// <returns>Zwraca uchwyt do pliku (NIE DZIALA Z FUNKCJAMI fprintf itp.). W przypadku niepowodzenia lub anulowania zwraca 0.</returns>
	int SelectAndSave(std::wstring ext = std::wstring(), std::wstring desc = std::wstring());
	
	/// <summary>Zamyka plik.</summary>
	void CloseFile();

	//Funkcja zapisuje dane do pliku - parametry jak dla funkcji printf.
	//Liczba znakow dla jednego wywolania funkcji ograniczona stala FILE_PRINTF_BUFFER_SIZE
	int printf(const TCHAR *format, ...);
	
	//Funkcja odczytuje dane z pliku - parametry jak dla funkcji scanf.
	//Ograniczenie do 8 parametrow.
	//Kazde wywolanie powoduje odczytanie calego wiersza i przejscie do nastepnego.
	//Liczba znakow dla jednego wywolania funkcji ograniczona stala FILE_SCANF_BUFFER_SIZE
	int scanf(const TCHAR *format, ...);
};

#endif