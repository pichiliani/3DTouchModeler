//===========================================================================
// Copyright (c) 1996-2012 Synaptics Incorporated. All rights reserved.
//
// SynCom API demo for ForcePad
//
// SynGestureDlg.cpp : implementation file
//
// Jan, 2012
//===========================================================================

#include "stdafx.h"
#include "SynForcePad.h"
#include "SynForcePadDlg.h"
#include "windows.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSynForcePadDlg dialog

CSynForcePadDlg::CSynForcePadDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CSynForcePadDlg::IDD, pParent)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_bConnected = FALSE;
  m_pAPI = NULL;
  m_pDevice = NULL;
  m_lDeviceHandle = -1;
}

void CSynForcePadDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSynForcePadDlg, CDialog)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  //}}AFX_MSG_MAP
  ON_WM_CLOSE()
END_MESSAGE_MAP()

// CSynForcePadDlg message handlers

BOOL CSynForcePadDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Set the icon for this dialog.  The framework does this automatically
  // when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);     // Set big icon
  SetIcon(m_hIcon, FALSE);    // Set small icon

  m_pAPI = new SynAPI;
  m_pDevice = new SynDevice;

  if (m_pAPI && m_pDevice) {
    // Add retry procedures in case one of driver components has not loaded yet
    // Connecting... if failed, repeat... until timeout
    for (int i = 0; i < 1000; i++) {
      if (Connect()) {
        break;
      }
      Sleep(100);
    }
  }
  
	aDados[0] = -1;
	aDados[1] = -1;
	aDados[2] = -1;
	aDados[3] = -1;
	aDados[4] = -1;
	
  
  
  return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.
void CSynForcePadDlg::OnPaint()
{
  if (IsIconic()) {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  } else {
    CDialog::OnPaint();
  }
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR CSynForcePadDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

// Register to SynCom
BOOL CSynForcePadDlg::Connect()
{
  if (!m_bConnected) {
    // Init...
    if ((*m_pAPI)->Initialize() == SYN_OK &&  // Initialize API
      // Find touchpad in USB port and get its device handle
      (*m_pAPI)->FindDevice(SE_ConnectionUSB, SE_DeviceTouchPad, &m_lDeviceHandle) == SYN_OK &&
      // Select device 
      (*m_pDevice)->Select(m_lDeviceHandle) == SYN_OK)
    {
      // Set event synchronization
      (*m_pAPI)->SetSynchronousNotification(this);
      (*m_pDevice)->SetSynchronousNotification(this);

      // Enable multifinger packets report
      (*m_pDevice)->SetProperty(SP_IsMultiFingerReportEnabled, 1);

      LONG lValue = 0;
      (*m_pDevice)->GetProperty(SP_HasMultiFingerPacketsGrouped, &lValue);
      m_bCanProcessGroups = !!lValue;
      (*m_pDevice)->GetProperty(SP_HasPacketGroupProcessing, &lValue);
      m_bCanProcessGroups = m_bCanProcessGroups && !!lValue;
      // Enable packet group processing (to include force)
      if (m_bCanProcessGroups) {
        (*m_pDevice)->SetProperty(SP_IsGroupReportEnabled, 1);
      }

      (*m_pDevice)->GetProperty(SP_NumMaxReportedFingers, &lValue);
      m_ulMaxGroupSize = (ULONG)lValue;
      (*m_pDevice)->GetProperty(SP_NumForceSensors, &lValue);
      m_ulNumForceSensors = (ULONG)lValue;
      ASSERT(m_ulNumForceSensors <= MAX_NUM_FORCESENSORS);

      m_bConnected = TRUE;

	  HANDLE hOut;
	CONSOLE_CURSOR_INFO ConCurInf;
 
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
 
	ConCurInf.dwSize = 10;
	ConCurInf.bVisible = FALSE;
 
	SetConsoleCursorInfo(hOut, &ConCurInf);


    } else {
      m_lDeviceHandle = -1;
      return FALSE;
    }
  }

  return TRUE;
}

// Unregister to SynCom
VOID CSynForcePadDlg::Disconnect()
{
  if (m_bConnected && m_lDeviceHandle != -1) {
    // Clear synchronization notifications
    (*m_pDevice)->SetSynchronousNotification(0);
    (*m_pAPI)->SetSynchronousNotification(0);
    m_bConnected = FALSE;
    m_lDeviceHandle = -1;
  }
}

// When device configuration is changed by adding/deleting device and power state changes
// device driver will send events notification to the application, which should be handled
// in the function OnSynAPINotify()
HRESULT CSynForcePadDlg::OnSynAPINotify(LONG lReason)
{
  switch (lReason) {
    case SE_DeviceRemoved:
      break;
    case SE_DeviceAdded:
      break;
    case SE_Configuration_Changed:
      Disconnect();
      // Reconnect to the API and the device due to possible configuration changes
      for (int i = 1; i < 1000; i ++) {
        if (Connect()) {
          return 0;
        }
        Sleep(100);
      }
      break;
    default:
      break;
  }
  return -1;
}

// Handle the packet notification
HRESULT CSynForcePadDlg::OnSynDevicePacket(LONG lSeq)
{

	int evento = 1;

   // evento = 1 -> TOUCH_DOWN
   // evento = 2 -> TOUCH_MOVE
   // evento = 3 -> TOUCH_UP



	if (m_bCanProcessGroups) {
    SynGroup grp;
    // display raw data
    if (SYN_OK == (*m_pDevice)->LoadGroup(grp)) {
      LONG lGroup = grp.GroupNumber();
      
      LONG lForceGrams[MAX_NUM_FORCESENSORS];
      LONG lForceRaw[MAX_NUM_FORCESENSORS];
      LONG lTotal = 0;
      for(ULONG ul = 0; ul < MAX_NUM_FORCESENSORS; ul++) {
        lForceGrams[ul] = grp.Force(ul);
        lForceRaw[ul] = grp.ForceRaw(ul);
        lTotal += lForceGrams[ul];
      }

      TCHAR tszBuffer[100] = {0};
      // only update every 10th packet group, for readability
      if (!(lGroup % 10)) {
        wsprintf(tszBuffer, TEXT("Force %d %d %d %d  grams: %d %d %d %d total(g): %d\n"),
          lForceRaw[0], lForceRaw[1], lForceRaw[2], lForceRaw[3],
          lForceGrams[0], lForceGrams[1], lForceGrams[2], lForceGrams[3], lTotal );
        SetDlgItemText(IDC_STATIC0, tszBuffer);
      }

      for (ULONG ul = 0; ul < m_ulMaxGroupSize; ul++) {
        SynPacket pkt;
        LONG lX, lY, lZ, lForce, lGroup;
        
        grp.Packet(ul, pkt);
        
        // lX = pkt.GetLongProperty(SP_XRaw);
		lX = pkt.GetLongProperty(SP_X);
        lY = pkt.GetLongProperty(SP_YRaw);
        lZ = pkt.GetLongProperty(SP_ZRaw);
        lForce = pkt.GetLongProperty(SP_ZForce);
        lGroup = pkt.GetLongProperty(SP_PacketGroupNumber);

		int x = (int) ul;

        TCHAR tszBuffer[100] = {0};
        // only update every 10th packet group, for readability
        if (!(lGroup % 10)) {
          if (lZ) {
             wsprintf(tszBuffer, TEXT("Finger %d, XRaw %d, YRaw %d, ZRaw %d, Force %d\n"),
               ul, lX, lY, lZ, lForce);

			if(aDados[x] == -1)
			{
				// Colocar o id no array, então é o evento TOUCH_DOWN
				aDados[x] = 1;
				// evento = 1;
				EviaDadosViaSocket(ul, lX, lY, lZ, lForce,1);

			}
			else
			{ 
				// JÁ EXISTE ENTÃOÉ O TOUCH_MOVE
				// evento = 2;
				aDados[x] = 2;
				EviaDadosViaSocket(ul, lX, lY, lZ, lForce,2);
			}

			// Mandando para a aplicação em Java os dados do sensor!
		    // EviaDadosViaSocket(ul, lX, lY, lZ, lForce,aDados[ul]);

          } else {
            
			// Só mando o TOUCH_UP se já tiver mandando m TOUCH_DOWN ou TOUCH_MOVE ANTES!
			if(aDados[x] != -1)
			{
				aDados[x] = -1;
				// RETIRO O DEDO ENTÃO É O EVENTO TOUCH_UP
				EviaDadosViaSocket(ul, 0, 0, 0, 0, 3);
			}
			 wsprintf(tszBuffer, TEXT("Finger %d off the pad.\n"), ul);

          }

          switch(ul) {
            case 0: SetDlgItemText(IDC_STATIC1, tszBuffer); break;
            case 1: SetDlgItemText(IDC_STATIC2, tszBuffer); break;
            case 2: SetDlgItemText(IDC_STATIC3, tszBuffer); break;
            case 3: SetDlgItemText(IDC_STATIC4, tszBuffer); break;
            case 4: SetDlgItemText(IDC_STATIC5, tszBuffer); break;
            default: break;
          }
        }

      }
    }
  } else {
      TCHAR tszBuffer[100] = {0};
      wsprintf(tszBuffer, TEXT("This driver installation does not support"));
      SetDlgItemText(IDC_STATIC2, tszBuffer);
      wsprintf(tszBuffer, TEXT("packet group and force processing.\n"));
      SetDlgItemText(IDC_STATIC4, tszBuffer);
  }

  return 0;
}


// Catch WM_CLOSE, some clean up before quit
VOID CSynForcePadDlg::OnClose()
{
  
 

  // Finalizações do dispositivo
  Disconnect();
  delete m_pDevice;
  delete m_pAPI;
  CDialog::DestroyWindow();
  PostQuitMessage(0);
}

void CSynForcePadDlg::EviaDadosViaSocket(ULONG ul,	LONG lX, LONG lY, LONG lZ, long lForce, int evento)
{

	   // evento = 1 -> TOUCH_DOWN
	   // evento = 2 -> TOUCH_MOVE
	   // evento = 3 -> TOUCH_UP
	
	
	  		// Aqui coloco as definições do SOCKET
		int wsaret=WSAStartup(0x101,&wsaData);
	
		if(wsaret)	
			return;
		
		conn=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		if(conn==INVALID_SOCKET)
			return;


		int porta = 123;

		sprintf(ServerNameInChar,"127.0.0.1");


		if(inet_addr((char*) ServerNameInChar)==INADDR_NONE)
		{
			hp=gethostbyname(ServerNameInChar);
		}
		else
		{
			addr=inet_addr(ServerNameInChar);
			hp=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
		}
		if(hp==NULL)
		{
			closesocket(conn);
			return;
		}

		server.sin_addr.s_addr=*((unsigned long*)hp->h_addr);
		server.sin_family=AF_INET;
		server.sin_port=htons(porta);

		if(connect(conn,(struct sockaddr*)&server,sizeof(server)))
		{
			closesocket(conn);
			return;	
		}

	    // Fim da inicialização do socket

	
		// SEQUENCIA DE DADOS: Finger %d, XRaw %d, YRaw %d, ZRaw %d, Force %d, evento
	// sprintf(buff,"Ola DADOS: Finger %d, XRaw %d, YRaw %d, ZRaw %d, Force %d",ul, lX, lY, lZ, lForce);
	// sprintf(buff,"Ola DADOS: Finger %d, X %d, Y %d, Z %d, F %d, E %d",ul, lX, lY, lZ, lForce, evento);

		sprintf(buff,"Ola %d,%d,%d,%d,%d,%d",ul, lX, lY, lZ, lForce, evento);
	send(conn,buff,strlen(buff),0);


	 // Finalizações do socket
 closesocket(conn);

 WSACleanup();

 
}

