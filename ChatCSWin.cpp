// ChatCSWin.cpp : Define el punto de entrada de la aplicación.
//

#include "pch.h"
#include "framework.h"
#include "ChatCSWin.h" 

////////////////////////////////////////////////
// Librerias para sockets
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "comctl32.lib")

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 		512
#define DEFAULT_PORT		"4200"


#define MAX_LOADSTRING 100

// Variables globales:
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal

char szMiIP[17] = "127.0.0.1";              // Dirección IP
char szUsuario[32] = "Carlos";              // Nombre actual de usuario 

DWORD WINAPI Servidor(LPVOID argumento);
int Cliente(HWND hChat, char* szDirIP, PSTR pstrMensaje);

void EnviarMensaje(HWND hChat, HWND hEscribir, HWND hIP);
void Mostrar_Mensaje(HWND hChat, char* szMiIP, char* szUsuario, char* szMsg, COLORREF color);
void Colorear_texto(HWND hChat, char* szUsuario, long iLength, COLORREF color);


// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Colocar código aquí.
    TCHAR tchIp[17], tchszUsuario[32];
    if (_tcslen(lpCmdLine) > 16) {
        swscanf(lpCmdLine, L"%s %s", tchIp, tchszUsuario);
        wcstombs(szMiIP, tchIp, 17);
        wcstombs(szUsuario, tchszUsuario, 32);

        /*sscanf(lpCmdLine, "%s %s", tchIp, tchszUsuario);
        sprintf(szMiIP, "%s", tchIp);
        sprintf(szUsuario, "%s", tchszUsuario);*/
    }

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHATCSWIN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realizar la inicialización de la aplicación:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHATCSWIN));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: Registra la clase de ventana.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHATCSWIN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CHATCSWIN);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: Guarda el identificador de instancia y crea la ventana principal
//
//   COMENTARIOS:
//
//        En esta función, se guarda el identificador de instancia en una variable común y
//        se crea y muestra la ventana principal del programa.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Almacenar identificador de instancia en una variable global

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 500, 450, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO: Procesa mensajes de la ventana principal.
//
//  WM_COMMAND  - procesar el menú de aplicaciones
//  WM_PAINT    - Pintar la ventana principal
//  WM_DESTROY  - publicar un mensaje de salida y volver
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hChat, hEscribir, hEnviar, hIP;
    HFONT hFont;
    static HANDLE hHiloServidor;
    static DWORD idHiloServidor;

    switch (message)
    {
    case WM_CREATE:
        {
        LoadLibrary(L"riched20.dll");
        hChat = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, L"", WS_CHILD
            | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_READONLY
            | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_EDITCHAT, hInst, NULL);

        hEscribir = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, L"", WS_CHILD
            | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL
            | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_EDITESCRIBIR, hInst, NULL);
            
        hIP = CreateWindowEx(0, 
            L"EDIT", 
            L"", 
            ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER | 
            WS_TABSTOP,
            10, 350, 
            350, 30,
            hWnd, 
            (HMENU)IDC_EDITIP, 
            hInst, 
            NULL);

        hEnviar = CreateWindowEx(0, L"BUTTON", 
            L"Enviar",
            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE |
            WS_BORDER | WS_TABSTOP,
            400, 350, 
            60, 30,
            hWnd, (HMENU)IDC_BOTONENVIAR, 
            hInst, 
            NULL);

        hFont = CreateFont(18, 0, 0, 0, 700, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial");

        InvalidateRect(hChat, NULL, TRUE);
        SendMessage(hChat, WM_SETFONT, (WPARAM)hFont, 0);
        SetFocus(hEscribir);

        hHiloServidor = CreateThread(
            NULL,
            0, 
            Servidor,
            (LPVOID)hChat,
            0,
            &idHiloServidor);

        if (hHiloServidor == NULL) {
            MessageBox(hWnd, L"Error al crear el hilo servidor", L"Error", MB_OK | MB_ICONERROR);
        }


        }
        break;

    case WM_SIZE: 
        {
        MoveWindow(hChat, 1, 1, LOWORD(lParam) - 1, HIWORD(lParam) - 140, TRUE);
        MoveWindow(hEscribir, 1, HIWORD(lParam) - 135, LOWORD(lParam) - 1, 60, TRUE);
        MoveWindow(hIP, 1, HIWORD(lParam) - 70, LOWORD(lParam) - 101, 30, TRUE);
        MoveWindow(hEnviar, LOWORD(lParam) - 70, HIWORD(lParam) - 70, 60, 30, TRUE);      
        }
         break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizar las selecciones de menú:
            switch (wmId)
            {
            case IDC_BOTONENVIAR:
                EnviarMensaje(hChat, hEscribir, hIP);
                
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Agregar cualquier código de dibujo que use hDC aquí...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        CloseHandle(hHiloServidor);       
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Controlador de mensajes del cuadro Acerca de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


int Cliente(HWND hChat, char* szDirIP, LPSTR pstrMensaje) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    char szMsg[256];                                      // Guarda la cadena de mensajes para entrada y salida
    char localhost[] = "localhost";
    char chat[] = "chat";
    TCHAR msgFalla[256];

    if (true) {

    //SetWindowText(hChat, szUsuario);
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wsprintf(msgFalla, L"WSAStartup failed with error: %d\n", iResult);
        MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);

        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolver la dirección y el puerto del servidor
    iResult = getaddrinfo(szDirIP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        wsprintf(msgFalla, L"getaddrinfo failed with error: %d\n", iResult);
        MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
        WSACleanup();

        return 1;
    }

    // Intente conectarse a una dirección hasta que una tenga éxito
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        // Crear el SOCKET para conectar al servidor
        if (ConnectSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"socket failed with error: %d\n", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
            WSACleanup();

            return 1;
        }

        // Connectar al server
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) { 
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"Unable to connect to server!\n", L"Error en  cliente", MB_OK | MB_ICONERROR);
        sprintf(szMsg, "Error en la llamada a connect\nla dirección %s no es válida", szDirIP);
        Mostrar_Mensaje(hChat, localhost, chat, szMsg, RGB(255, 0, 0));
        WSACleanup();

        return 1;
    }
    }
    


    /******     Envio de mensajes al servidor       *******/

    sprintf(szMsg, "%s %s", szDirIP, szUsuario);

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Enviar IP y nombre de Usuario
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Recibir confirmación de conexión "ok"

    strcpy(szMsg, pstrMensaje);                                             // Cargar Mensaje de chat

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Enviar Mensaje
    iResult = shutdown(ConnectSocket, SD_SEND);
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Recibir el mismo mensaje enviado

    //sprintf(prueba, "ojo %s", szMsg);
    //SetWindowText(hChat, prueba);

    Mostrar_Mensaje(hChat, szMiIP, szUsuario, szMsg, RGB(0, 0, 255));       // Imprimir Mensaje en el editor dersación

    WCHAR    str[256];
    const char *ab = "0 0 1";    
    MultiByteToWideChar(0,0,szMsg,256,str,256);    
    MessageBox(NULL, str, L"AAAAA", MB_OK);


    closesocket(ConnectSocket);                                             // Cerrar el socket
    WSACleanup();

    return 1;
}



DWORD WINAPI Servidor(LPVOID argumento) {
    HWND hChat = (HWND)argumento;
    WSADATA wsaData;
    int iResult;
    TCHAR msgFalla[256];

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;
    char szBuffer[256], szIP[16], szNN[32];

    if (true) {


    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wsprintf(msgFalla, L"WSAStartup failed with error: %d\n", iResult);
        MessageBox(NULL, msgFalla, L"Error em servidore", MB_OK | MB_ICONERROR);

        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        wsprintf(msgFalla, L"getaddrinfo failed with error: %d", iResult);
        MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
        WSACleanup();

        return 1;
    }

    // Crear SOCKET para la coneccion al server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        wsprintf(msgFalla, L"socket failed with error: %d", WSAGetLastError());
        MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
        freeaddrinfo(result);
        WSACleanup();

        return 1;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        wsprintf(msgFalla, L"listen failed with error: %d\n", WSAGetLastError());
        MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        wsprintf(msgFalla, L"listen failed with error: %d", WSAGetLastError());
        MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }
    }

    while (TRUE) {
        // MessageBox(NULL, L"Esperando conexión", L"Depuración", MB_OK);
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"acept failed with error: %d", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        /*****      Enviar mensajes al cliente     ******/

        // Recibir hasta que el par cierra la conexión

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        sscanf(szBuffer, "%s %s", szIP, szNN);
        sprintf_s(szBuffer, "Ok");

        // Enviar el búfer al remitente

        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);

        Mostrar_Mensaje(hChat, szIP, szNN, szBuffer, RGB(34, 177, 76));

        WCHAR    str[256];
        MultiByteToWideChar(0, 0, szBuffer, 256, str, 256);
        MessageBox(NULL, str, L"BBBB", MB_OK);


        iResult = shutdown(ClientSocket, SD_SEND);
    }

    // Limpiar

    closesocket(ClientSocket);
    WSACleanup();

    return 1;
}

void Mostrar_Mensaje(HWND hChat, char* ip, char* szUsuario, char* szMsg, COLORREF color) {
    long iLength;
    char* pstrBuffer = NULL;
    TCHAR* ptchBuffer = NULL;
    DWORD iIni = 0, iFin = 0;
    POINT posScroll;
    int r = 0;
    size_t i;

    int tam = strlen(szUsuario) + strlen(ip) + strlen(szMsg);

    if ((pstrBuffer = (PSTR)malloc(sizeof(char) * (tam + 10))) == NULL ||
        (ptchBuffer = (TCHAR*)malloc(sizeof(TCHAR) * (tam + 10))) == NULL)
        MessageBox(NULL, L"Error al reservar memoria", L"Error", MB_OK | MB_ICONERROR);

    sprintf_s(pstrBuffer, tam + 9, "%s -> %s\n%s\n", szUsuario, ip, szMsg);
    //sprintf(pstrBuffer, "%s -> %s\n%s\n", szUsuario, ip, szMsg);
    tam = strlen(pstrBuffer);
    mbstowcs_s(&i, ptchBuffer, tam+1, pstrBuffer, tam + 1);


    /*******************************************************************************/

    //SetWindowText(hChat, pstrBuffer);

    iLength = GetWindowTextLength(hChat);
    r = SendMessage(hChat, EM_GETLINECOUNT, 0, 0) - 1;                              // Contador de saltos de línea

    SendMessage(hChat, EM_SETSEL, (WPARAM)iLength, (LPARAM)iLength + tam);          // Se establece un rango de texto a seleccionar
    SendMessage(hChat, EM_REPLACESEL, FALSE, (LPARAM)ptchBuffer);

    Colorear_texto(hChat, szUsuario, iLength - r, color);

    SetFocus(hChat);                                                            // Posíciona el cursor en el editor grande
    SendMessage(hChat, EM_GETSEL, (WPARAM)&iIni, (LPARAM)&iFin);                // Posición actual del cursor
    SendMessage(hChat, EM_GETSCROLLPOS, 0L, (LPARAM)&posScroll);
    SendMessage(hChat, EM_SETSEL, (WPARAM)iIni, (LPARAM)iFin);                  // Reposicionar el cursor
    SendMessage(hChat, EM_SETSCROLLPOS, 0L, (LPARAM)&posScroll);

    free(pstrBuffer);
    free(ptchBuffer);
}

void Colorear_texto(HWND hChat, char* szUsuario, long iLength, COLORREF color) {
    CHARFORMAT2 cf;                                                                         // Formato del texto
    size_t i;

    // El siguiente segmento de codigo da formato reemplazando texto
    // normal por texto con formato

    memset(&cf, 0, sizeof cf);                  // Se limpia la estructura del formato
    cf.cbSize = sizeof(CHARFORMAT2);            // Se fija el tamaño de la esctructura
    cf.dwMask = CFM_COLOR;                      // Se establece la mascara para que sea posible aplicar color al texto
    cf.crTextColor = color;

    TCHAR auxiliar[35];

    /*
    if ((auxiliar = (LPSTR)malloc(sizeof(char) * (35 + 2))) == NULL)
        MessageBox(NULL, L"Error al reservar memoria", L"Error", MB_OK | MB_ICONERROR);*/

    int tam = strlen(szUsuario);
    mbstowcs_s(&i, auxiliar, szUsuario, sizeof(szUsuario) + 2);                 // Empatar(lexema)
    //strcpy(auxiliar, szUsuario);

    SendMessage(hChat, EM_SETSEL, (WPARAM)iLength, (LPARAM)iLength + tam);      // Se establece un rango de texto a selecionar
    SendMessage(hChat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);           // Se aplica el formato al rango seleccionado
    SendMessage(hChat, EM_REPLACESEL, FALSE, (LPARAM)auxiliar);                 //Se reemplaza el rango seleccionado con el nuevo texto
    cf.crTextColor = RGB(0, 0, 0);
}


void EnviarMensaje(HWND hChat, HWND hEscribir, HWND hIP) {
    TCHAR tchDirIP[16];
    char szDirIP[16];
    int tam = 0;
    size_t i;
/*
    if ((tchDirIP = (LPSTR)malloc(sizeof(char) * (16 + 2))) == NULL)
        MessageBox(NULL, L"Error al reservar memoria", L"Error", MB_OK | MB_ICONERROR);
*/
    GetWindowText(hIP, tchDirIP, 16);           // Copiar el contenido de la caja de texto
    tam = GetWindowTextLength(hIP);             // Tamaño de la cadena
    wcstombs(szDirIP, tchDirIP, tam);          // Conversion de TCHAR a char
    szDirIP[tam] = '\0';                       // Fin de cadena

    long iLength;
    PSTR pstrBuffer;
    TCHAR* ptchBuffer;

    iLength = GetWindowTextLength(hEscribir);
    if (NULL == (pstrBuffer = (PSTR)malloc(sizeof(char) * (iLength + 2)))||
        NULL == (ptchBuffer = (TCHAR*)malloc(sizeof(TCHAR) * (iLength + 2))))
        MessageBox(NULL, L"Error al reservar memoria", L"Error", MB_OK | MB_ICONERROR);
    else {
        GetWindowText(hEscribir, ptchBuffer, iLength + 1);     // Copiamos lo que tiene la caja de mensjes a enviar
        wcstombs_s(&i, pstrBuffer, (iLength + 1), ptchBuffer, (iLength + 1)); 
        pstrBuffer[iLength + 1] = '\0';

        Cliente(hChat, szDirIP, pstrBuffer);

        SetWindowText(hEscribir, L"");
        free(pstrBuffer);
        free(ptchBuffer);
    }
}