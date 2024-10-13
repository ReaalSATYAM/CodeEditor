#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

// Command Id
#define Id_new_file 1
#define Id_open_file 2
#define Id_close_file 3
#define Id_save_file 4
#define Id_save_file_as 5
#define Id_run 8
#define HOTKEY_CTRL_S 9 
#define Id_Credits 10
#define fontSize 30

//Global variables
static HWND TextBox;  
char fileName[100];
static int lineCount;
HFONT hFont; 
HWND hStatic;

//Function declaration
LRESULT CALLBACK windowprocedure(HWND, UINT, WPARAM, LPARAM);
void AddMenu(HWND);
void OnButtonClick(HWND );
void compiler(HWND hnd, wchar_t *code);
void AddTextToTextBox(HWND hnd, const wchar_t *text);
void openFile(HWND hnd, HWND TextBox);
void displayFile(char *path);
void saveFileAs(HWND hnd, HWND TextBox);
void newFile(HWND hnd, HWND TextBox);
void save(HWND hnd);
int extentionChecker();
void lineNumAdder(HWND hnd , HWND hStatic);
void token(char *fContent, FILE *fp);
void SetTabSize(HWND hTextBox, int tabSize);


 
int WINAPI WinMain(HINSTANCE hint, HINSTANCE hprev, LPSTR agr, int ncmd)
{
    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1); 
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hint;
    wc.lpszClassName = L"window";
    wc.lpfnWndProc = windowprocedure;

    if (!RegisterClassW(&wc))
        return -1;
        
    // main window
    HWND hnd = CreateWindowW(L"window", L"Editor", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                  200, 40, 800, 700, NULL, NULL, hint, NULL);


    MSG msg = {0};
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window procedure function definition
LRESULT CALLBACK windowprocedure(HWND hnd, UINT msg, WPARAM wp, LPARAM lp)
{
    // Get the new width and height of the window
    int width = LOWORD(lp);
    int height = HIWORD(lp);
    int lineWidth = 45;

    switch (msg)
    {
    case WM_CREATE:
        AddMenu(hnd);
        
        //Text box
        TextBox = CreateWindowExW(
            WS_EX_CLIENTEDGE,       
            L"Edit",               
            NULL,                   
            WS_CHILD | WS_VISIBLE |WS_VSCROLL| ES_MULTILINE | ES_AUTOVSCROLL, 
            100, 10,               
            800, 700,               
            hnd,                    
            NULL,                   
            NULL,                   
            NULL                    
        );

        //default text 
        SetWindowTextW(TextBox, L"#include<stdio.h>\r\nint main(){\r\n\tprintf(\"Hello World\");\r\n\treturn 0;\r\n}");
        hFont = CreateFontW(
            fontSize,                       
            0,                         
            0,                         
            0,                        
            FW_NORMAL,                   
            FALSE,                     
            FALSE,                     
            FALSE,                     
            DEFAULT_CHARSET,           
            OUT_DEFAULT_PRECIS,        
            CLIP_DEFAULT_PRECIS,       
            DEFAULT_QUALITY,           
            DEFAULT_PITCH | FF_SWISS,  
            L"Arial");        
        
      
        SendMessage(TextBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SetTabSize(TextBox, 15);

        //static field
        hStatic = CreateWindowW(L"Edit", NULL,
                                WS_CHILD | WS_VISIBLE |WS_VSCROLL| ES_MULTILINE | ES_AUTOVSCROLL |ES_READONLY|SS_CENTER,
                                0, 2, lineWidth,  fontSize*height, hnd, NULL, NULL, NULL);
                                wchar_t *textToDisplay = L"";

        SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
        SetWindowTextW(hStatic, textToDisplay); 


        if (!RegisterHotKey(hnd, HOTKEY_CTRL_S, MOD_CONTROL, 'S'))
        {
            MessageBoxW(hnd, L"Failed to register Ctrl + S hotkey", L"Error", MB_OK);
        }


        break;

    //COMMANDS
    case WM_COMMAND:

        // Menu Actions
        switch (LOWORD(wp))
        {
        case Id_new_file: // New
            newFile(hnd, TextBox);
            break;
        case Id_open_file: // Open File
            openFile(hnd, TextBox );
            break;

        case Id_save_file: // Save
            save(hnd);
            break;
        case Id_save_file_as: // Save As
            saveFileAs(hnd, TextBox);
            break;
        case Id_close_file: // Exit
            DestroyWindow(hnd); 
            break;
        case Id_run://compile & run
            OnButtonClick(hnd);
            break;
        case Id_Credits:
            MessageBoxW(hnd, 
                    L"Developed by: Satyam Naithani\n"
                    L"LinkedIn: https://www.linkedin.com/in/satyam-naithani-243076298/\n"
                    L"GitHub: https://github.com/ReaalSATYAM",
                    L"Credits",
                    MB_OK );
            break;
        }
        break;

    // auto adjust text box size
    case WM_SIZE:
    {
        SetWindowPos(hStatic, NULL, 0, 0, lineWidth , height, SWP_NOZORDER);
        MoveWindow(TextBox, lineWidth, 0, width - lineWidth, height, TRUE);
        SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    }

    case WM_HOTKEY:
        if (wp == HOTKEY_CTRL_S)
           save(hnd);

        break;

    case WM_CLOSE:
        save(hnd);
        DestroyWindow(hnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hnd, msg, wp, lp);
    }
    return 0;
}

void AddMenu(HWND hnd)
{
    HMENU hMenuBar;
    HMENU hMenu;
    HMENU subMenu;

    hMenuBar = CreateMenu();
    hMenu = CreateMenu();

    //Menu items for file
    AppendMenuW(hMenu, MF_STRING, Id_new_file, L"New");
    AppendMenuW(hMenu, MF_POPUP, Id_open_file, L"Open file");
    AppendMenuW(hMenu, MF_STRING, Id_save_file, L"Save");
    AppendMenuW(hMenu, MF_STRING, Id_save_file_as, L"Save As");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, Id_Credits, L"Credits");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, Id_close_file, L"Exit");
    

    // Menu bar
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hMenu, L"&File");
    AppendMenuW(hMenuBar, MF_STRING, Id_run, L"&Run");

    SetMenu(hnd, hMenuBar);
}


void OnButtonClick(HWND hnd)
{
    int textLength = GetWindowTextLengthW(TextBox);

    wchar_t *code = (wchar_t *)malloc((textLength + 1) * sizeof(wchar_t)); 
    if(code)
    {
        GetWindowTextW(TextBox, code, textLength + 1);
        compiler(hnd, code);
        free(code);
    }
    else{
        MessageBoxW(hnd,L"Memory allocation failed!", L"Alert", MB_OK);
    }
}

void compiler(HWND hnd, wchar_t *code)
{
    save(hnd);
    lineNumAdder(hnd, hStatic);
    char *tempFileName;
    int ext = extentionChecker();
    WCHAR command[100];
    switch(ext)
    {
        case 1: 
            tempFileName = "tempCodeRunnerC.c";
            wcscpy(command, L"cmd.exe /k gcc tempCodeRunnerC.c -o tempCodeRunnerC.exe && tempCodeRunnerC.exe");
            break;
        case 2: 
            tempFileName = "tempCodeRunnerPy.py";
            wcscpy(command, L"cmd.exe /k python tempCodeRunnerPy.py");
            break;
        case 3:  
            tempFileName = "tempCodeRunnerCpp.cpp";
            wcscpy(command, L"cmd.exe /k g++ tempCodeRunnerCpp.cpp -o tempCodeRunnerCpp.exe && tempCodeRunnerCpp.exe");
            break;
        default:
                MessageBoxW(hnd,L"Invalid file extention", L"Alert", MB_OK);
                break;
    }
    FILE *fp = fopen(tempFileName, "w");
    lineCount = 1;
    if(fp)
    {
        char* multibyteCode = (char*)malloc((wcslen(code) + 1) * sizeof(char));
        wcstombs(multibyteCode, code, wcslen(code) + 1);

        char *ptr = multibyteCode;
        while(*ptr != '\0'){
        if(*ptr == '\r'){
            ptr++;
            lineCount++;
        }
        else if(*ptr == '\n'){
            fputc('\n', fp);
            ptr++;
        }
        else{
            fputc(*ptr, fp);
            ptr++;
        }
    }
        fclose(fp);
        free(multibyteCode);
        STARTUPINFOW si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);

        if (CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
    else{
        MessageBoxW(hnd, L"Failed to save file!", L"Error", MB_OK | MB_ICONERROR);
    }
}

void openFile(HWND hnd, HWND TextBox){

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hnd;
    ofn.lpstrFile = fileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 100;
    ofn.lpstrFilter = "All files\0*.*\0Text files\0*.txt\0C files\0*.c\0C++ files\0*.cpp\0Python files\0*.py\0";
    ofn.nFilterIndex = 1;
    
    GetOpenFileName(&ofn);
    
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        MessageBoxW(NULL, L"Failed to open file", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    wchar_t existingContent[10000]= L""; 
    char ch;

    while ((ch = fgetc(fp)) != EOF) {
        wchar_t wc[2] = {0};
        if (ch == '\n') {
            wcscat(existingContent, L"\r\n"); 
        } else {
            MultiByteToWideChar(CP_UTF8, 0, (const char *)&ch, 1, wc, 1);
            wcscat(existingContent, wc);
        }
    }

    SetWindowTextW(TextBox, existingContent);
    fclose(fp);
}


void saveFileAs(HWND hnd, HWND TextBox){
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hnd;
    ofn.lpstrFile = fileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 100;
    ofn.lpstrFilter = "All files\0*.*\0Text files\0*.txt\0C files\0*.c\0C++ files\0*.cpp\0Python files\0*.py\0";
    ofn.nFilterIndex = 1;
    
    GetSaveFileName(&ofn);
    FILE *fp = fopen(fileName, "w");
    if (fp == NULL) {
        MessageBoxW(NULL, L"Failed to save file", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    int size = GetWindowTextLength(TextBox);
    char *fContent = (char *)malloc((size + 1) * sizeof(char));
    if (fContent == NULL) {
        MessageBoxW(NULL, L"Memory allocation failed", L"Error", MB_OK | MB_ICONERROR);
        fclose(fp);
        return;
    }
    GetWindowText(TextBox, fContent, size + 1);
    lineCount = 1;
    char *ptr = fContent;
    while(*ptr != '\0'){
        if(*ptr == '\r'){
            ptr++;
            lineCount++;
        }
        else{
            fputc(*ptr, fp);
            ptr++;
        }
    }
    free(fContent);
    fclose(fp);

}
void newFile(HWND hnd, HWND TextBox){

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hnd;
    ofn.lpstrFile = fileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 100;
    ofn.lpstrFilter = "All files\0*.*\0Text files\0*.txt\0C files\0*.c\0C++ files\0*.cpp\0Python files\0*.py\0";
    ofn.nFilterIndex = 1;

    GetSaveFileName(&ofn);

    int size = GetWindowTextLength(TextBox);
    FILE *fp = fopen(fileName, "w");
    if (fp == NULL) {
        MessageBoxW(NULL, L"Failed to save file", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    fputs("",fp);
    //fclose(fp);

    char *fContent = (char *)malloc((size + 1) * sizeof(char));
    if (fContent == NULL) {
        MessageBoxW(NULL, L"Memory allocation failed", L"Error", MB_OK | MB_ICONERROR);
        fclose(fp);
        return;
    }

    wchar_t existingContent[10000]= L""; 
    wchar_t *newContent;  

    while (fgets(fContent, size, fp) != NULL)
    {
        int wideCharSize = MultiByteToWideChar(CP_UTF8, 0, fContent, -1, NULL, 0);
        newContent = (wchar_t *)malloc(wideCharSize * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, fContent, -1, newContent, wideCharSize);

        wcscat(existingContent, newContent);
        wcscat(existingContent, L"\r\n"); 
        
        free(newContent);
    }
    SetWindowTextW(TextBox, existingContent);
    
    fclose(fp);
    free(fContent);
}


void save(HWND hnd)
{
    if(fileName[0] == '\0')
        saveFileAs(hnd, TextBox);
    FILE *fp = fopen(fileName, "w");
    if (fp == NULL) {
        MessageBoxW(NULL, L"Failed to save file", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    int size = GetWindowTextLength(TextBox);
    char *fContent = (char *)malloc((size + 1) * sizeof(char));
    if (fContent == NULL) {
        MessageBoxW(NULL, L"Memory allocation failed", L"Error", MB_OK | MB_ICONERROR);
        fclose(fp);
        return;
    }
    GetWindowText(TextBox, fContent, size + 1);
    lineCount = 1;
    char *ptr = fContent;
    while(*ptr != '\0'){
        if(*ptr == '\r'){
            ptr++;
            lineCount++;
        }
        else{
            fputc(*ptr, fp);
            ptr++;
        }
    }
    free(fContent);
    fclose(fp);
    
}

int extentionChecker(){
    char *ext = strrchr(fileName, '.'); 
    if (ext != NULL && strcmp(ext, ".c") == 0)
        return 1;
    else if(ext != NULL && strcmp(ext, ".py")==0)
        return 2;
    else if(ext != NULL && strcmp(ext, ".cpp")==0)
        return 3;
}


void lineNumAdder(HWND hnd , HWND hStatic){
    wchar_t textToDisplay[1000] = L""; 
    wchar_t lineBuffer[20];            

    for(int i = 1; i <= lineCount; i++) {
        swprintf(lineBuffer, sizeof(lineBuffer) / sizeof(wchar_t), L"%d\r\n", i); 
        wcscat(textToDisplay, lineBuffer); 
    }

    SetWindowTextW(hStatic, textToDisplay); 
    SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// Function to set tab size for the TextBox
void SetTabSize(HWND hTextBox, int tabSize) {
    int tabStops[1];
    tabStops[0] = tabSize; // Set the tab size

    SendMessage(hTextBox, EM_SETTABSTOPS, 1, (LPARAM)tabStops);
}