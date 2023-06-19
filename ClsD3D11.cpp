#include "ClsD3D11.h"

namespace D3D
{
	/// <summary>
	/// Constructor
	/// Erhält notwendige Daten um Fenster bzw. Inhalt in richtiger Größe anzuzeigen
	/// Erhält das WndHandle
	/// Mit ReleaseAndGetAdressOf() werden die Variablen auf NULL gesetzt
	/// </summary>
	/// <param name="pClsDataContainer">Container der alle notwendigen Daten für Fenster enthält</param>
	ClsD3D11::ClsD3D11()
	{
		m_hWnd = NULL;
		m_uiPickedMonitor = MAINMONITOR;
		m_myClientRect = {};
		m_myCpyMethod = DEFCPYMETHOD;
		m_pFrameData = NULL;

		// Device, Factory, SwapChain
		m_pD3dDevice.ReleaseAndGetAddressOf();
		m_pD3dContext.ReleaseAndGetAddressOf();
		m_pDxgiDevice2.ReleaseAndGetAddressOf();
		m_pdxgiAdapter.ReleaseAndGetAddressOf();
		m_pdxgiFactory2.ReleaseAndGetAddressOf();

		// SwapChain
		m_pDxgiSwapChain.ReleaseAndGetAddressOf();
		m_pTarRenderView.ReleaseAndGetAddressOf();
		m_pBackBuffer.ReleaseAndGetAddressOf();

		// Displaying Texture
		m_pD3D11Texture.ReleaseAndGetAddressOf();
		m_pShaderView.ReleaseAndGetAddressOf();
		
		// Shader Programs
		m_pVertexShaderBlob.ReleaseAndGetAddressOf();
		m_pPixelShaderBlob.ReleaseAndGetAddressOf();

		// Includes the Buffer, Description and Indices 
		// for the 2 Displaying triangles. Its the RECT for the Window
		m_myIndexBuffer = {};

		// DesktopDupl
		m_pDeskDupl.ReleaseAndGetAddressOf();
		m_pDxgiDesktopResource.ReleaseAndGetAddressOf();
		m_pD3dAcquiredDesktopImage.ReleaseAndGetAddressOf();
		m_pD3D11TextureCPUAccess.ReleaseAndGetAddressOf();
		m_myTextureDescCPUAccess = {};
		m_MyFrameInfo = {};
		m_mySubResD3D11Texture = {};
		
		// D2D1 Interface
		m_pClsD2D1 = NULL;

		m_myConstantBuffer = {};
	}
	/***********Set-Get-Methodes******/
	void ClsD3D11::SetFrameData(FrameData** ppFrameData)
	{
		m_pFrameData = *ppFrameData;
	}
	void ClsD3D11::SetWnd(HWND hWnd)
	{
		m_hWnd = hWnd;
	}

	void ClsD3D11::SetCpyMethod(CopyMethod& myCpyMethod)
	{
		m_myCpyMethod = myCpyMethod;
	}

	void ClsD3D11::SetPickedMonitor(UINT uiPickedMonitor)
	{
		m_uiPickedMonitor = uiPickedMonitor;
	}
	void ClsD3D11::SetClientRect(RECT myClientRect)
	{
		m_myClientRect = myClientRect;
	}
	/*********************************/
	/// <summary>
	/// Init. the D2D1 Surface. Will be used if CopyMethod::D2D1Surface
	/// Overlay a 2D-Picture over the D3D-Content
	/// CalledBy: CreateD3DTexture
	/// </summary>
	/// <returns></returns>
	HRESULT ClsD3D11::InitD2D1()
	{
		HRESULT hr = NULL;
		m_pClsD2D1 = new D3D::ClsD2D1(m_pD3D11Texture,m_hWnd);
		
		if(!m_pClsD2D1)
			HRESULT_FROM_WIN32(GetLastError());
		m_pClsD2D1->SetFrameData(&m_pFrameData);
		HR_RETURN_ON_ERR(hr, m_pClsD2D1->InitD2D1RT());
		HR_RETURN_ON_ERR(hr, m_pClsD2D1->CreateBitmapForD2D1RT());

		return hr;
	}//END-FUNC

	/// <summary>
	/// Erstellt eine gültiges Device und DeviceContext.
	/// Liefert somit gültige Adresse zum Pointer der wiederum auf ein erstelltes ComObj zeigt.
	/// Dies wird für Device und DeviceContext erledigt
	/// </summary>
	HRESULT ClsD3D11::CreateDevice()
	{
		HRESULT hr = NULL;										// Rückgabewert von ApiFkt. für Errorhandling
		ComPtr<IDXGIDevice2> dxgiDevice;						// Smartpointer zum COM-DXGIDevice-Interface-Obj
		D3D_FEATURE_LEVEL pSelectedFeatureLvl;					// will recieve the FeatureLvl set by GPU
		D3D_FEATURE_LEVEL aFeatureLevels[] =					// FeatureLvlArray, bricht ab nachdem er eins erfolgreich bestätigt hat
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_0
		};

		UINT iSizeFeaturLvl = ARRAYSIZE(aFeatureLevels);		// Size of FeatureLvl Array
		/*Übergibt alle gesetzten Parameter und speichert im pD3dDevice eine Adresse zum Pointer des DeviceObj was über das COM-IF erstellt wurde
		* Device: Obj für die Erstellung sämtliche Obj bzgl DX
		* Context: Zur Manipulation sämtlicher erstellten Obj von Device*/
		HR_RETURN_ON_ERR(hr, D3D11CreateDevice(
			NULL,												// Adapter, auszulesen über D3D11Adapter->EnumAdapters oder NULL: DefaultAdapter
			D3D_DRIVER_TYPE_HARDWARE,							// Hardwarebeschleunigung der GPU nutzen
			NULL,												// Handle falls Software genutzt wird (CPU statt GPU)
			D3D11_CREATE_DEVICE_BGRA_SUPPORT |					// Mit BGRA support (Blue,Green,red,alpha) 
			D3D11_CREATE_DEVICE_DEBUG,							// zusätzliche DebugMsgs im Compilerfenster
			aFeatureLevels,										// Array mit FeatureLvl,von 0 bis n, bricht bei Erfolg ab
			ARRAYSIZE(aFeatureLevels),							// Größe des FeatureLvlArrays
			D3D11_SDK_VERSION,									// Installierte DX SDK
			&m_pD3dDevice,										// wird Adresse zum Pointer des DeviceIFObjs speichern
			&pSelectedFeatureLvl,								// recieved FeatureLvl
			&m_pD3dContext										// wird Adresse zum Pointer des DeviceContextIFObjs speichern
		));

		HR_RETURN_ON_ERR(hr, m_pD3dDevice->QueryInterface(IID_PPV_ARGS(&m_pDxgiDevice2)));	// castet pD3dDevice-Obj in dxgiDevice
		HR_RETURN_ON_ERR(hr, m_pDxgiDevice2->GetAdapter(&m_pdxgiAdapter));						// Pointer zum Adapter-COM-IF-Obj
		HR_RETURN_ON_ERR(hr, m_pdxgiAdapter->GetParent(IID_PPV_ARGS(&m_pdxgiFactory2)));		// Addresse  des Pointers zum ElternObj von Adapter  

		return hr;
	}
	/*
	void ClsD3D11::GetMonitorList()
	{
		UINT uiCount = 0;
		IDXGIOutput* pOutput;
		DXGI_OUTPUT_DESC myMonitorDesc;
		
		while (m_pdxgiAdapter->EnumOutputs(uiCount, &pOutput) != DXGI_ERROR_NOT_FOUND)
		{
			MonitorInfo* pMonitorInfo = new MonitorInfo();
			//pMonitorInfo->pOutput = pOutput;
			pOutput->GetDesc(&myMonitorDesc);
			pMonitorInfo->myMonitorScreen = myMonitorDesc.DesktopCoordinates;

			pMonitorInfo->iWidth = myMonitorDesc.DesktopCoordinates.right - myMonitorDesc.DesktopCoordinates.left;
			pMonitorInfo->iHeight = myMonitorDesc.DesktopCoordinates.bottom - myMonitorDesc.DesktopCoordinates.top;

			pMonitorInfo->iWidth = (pMonitorInfo->iWidth < 0) ? pMonitorInfo->iWidth * (-1) : pMonitorInfo->iWidth;
			pMonitorInfo->iHeight = (pMonitorInfo->iHeight < 0) ? pMonitorInfo->iHeight * (-1) : pMonitorInfo->iHeight;

			wcsncpy_s(pMonitorInfo->wstrDeviceName, myMonitorDesc.DeviceName, sizeof(myMonitorDesc.DeviceName));
			pMonitorInfo->uiMonitor = uiCount;
			m_vMonitorInfo.push_back(pMonitorInfo);
			++uiCount;
		}
		m_uiMaxMonitors = --uiCount;
	}
	void ClsD3D11::SetDisplay(UINT uiMonitorID)
	{
		if (uiMonitorID > m_uiMaxMonitors)
			m_uiPickedMonitor = 0;
		else
			m_uiPickedMonitor = uiMonitorID;

		m_pClsDataContainer->SetResolution(
			m_vMonitorInfo[m_uiPickedMonitor]->iWidth,
			m_vMonitorInfo[m_uiPickedMonitor]->iHeight,
			BITDEPTH);
	}*/
	/// <summary>
	/// - Erstellt eine Swapchain1 anstatt Swapchain
	/// - Für Swapchain1 ist eine Factory notwendig: 
	///		- pD3dDevice auf dxgiDevice. Polymorphie des Ziels unseres Pointers von pD3dDevice
	///		- Erstellt über dxgiDevice ein COM-IF-OBj für Adpater
	///		- Parent von dxgiAdapter ist dxgiFactory
	///  mit übergebenen Device
	/// Referenz zum leeren ComPtr für den Adapter
	/// Referenz zum leeren ComPtr für Swapchain</summary>
	HRESULT ClsD3D11::CreateSwapChain()
	{
		HRESULT hr = NULL;											// Rückgabewert von ApiFkt. für Errorhandling
		//ComPtr<IDXGIDevice2> dxgiDevice;							// Smartpointer bzw Adresse zum COM-Factory-Interface-Obj

		DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};				// typed Struct für die SwapchainDescription

		d3d11SwapChainDesc.Width = 0;								// use window width = 0
		d3d11SwapChainDesc.Height = 0;								// use window height = 0
		d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;// BGRA Format, BGRA ist StandardFormat für die Desktopdubli.
		d3d11SwapChainDesc.SampleDesc.Count = 1;					// Kantenglättung. Wieviel Pixel für ein Pixel für Glättung: 1Px für ein Px = aus
		d3d11SwapChainDesc.SampleDesc.Quality = 0;					// AntiAliasing bzw. Kantenglättung ist aus, folgt Quality 0
		d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Wie wird Buffer genutzt
		d3d11SwapChainDesc.BufferCount = 1;							// 1 Backbuffer und 1 Frontbuffer
		d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;			// DXGI passt BackbufferGröße an Ziel an 
		d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;//DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;// DXGI_SWAP_EFFECT_DISCARD;	// kopiert den Buffer mit jedem Call von device->Draw in den WindowDesktopManager
																	// DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL shared mit WDM, jederzeit Zugriff ohne CopyOperation
		d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // Alphakanal/Transparenz nicht definiert
		d3d11SwapChainDesc.Flags = 0;								// z.B.: DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE erlaubt das GDI in DXI rendert		

		/*
		* &ComPtr Adresse des ComPtr Obj selbst
		* ComPtr.get() Adresse des Ziels vom IF-Pointer
		* ComPtr.getadressof() Adresse des IF-Pointers selbst
		* &ComPtr genauso wie .getadressof() mit dem Unterschied, dass das Ziel des Pointers zurückgesetzt wird. (Neues Obj wird init.)
		*/
		HR_RETURN_ON_ERR(hr, m_pdxgiFactory2->CreateSwapChainForHwnd(
			m_pD3dDevice.Get(),								// Ziel des Pointers, dass Wiederum ein Pointer auf ein COM-IF-Obj ist
			m_hWnd,											// globaler Handle zu meinem Window
			&d3d11SwapChainDesc,							// Swapchain Description per Addresse
			nullptr,										// Description für Fullscreen
			NULL,											// Falls FullscreenDescription, hier das COMIF-Obj für den Monitor
															// per dxgiAdapter->EnumOutput(..)
			&m_pDxgiSwapChain								// Speichert hier die Adresse des Ziels, was ein Pointer zum COM-IF-Obj ist
		));

		return hr;
	}//END-FUNC CreateSwapChain

	/// <summary>
	/// Alloctae Buffer that will be used for the Swapchain
	/// Binds Buffer on RTView, View is binded on the Pipeline
	/// </summary>
	HRESULT ClsD3D11::CreateSwapChainBuffer()
	{
		HRESULT hr = NULL;
		HR_RETURN_ON_ERR(hr, m_pDxgiSwapChain
			->GetBuffer(0, IID_PPV_ARGS(&m_pBackBuffer)));	// 1:param: 0 für BackBuffer, 2.: Adresse des BackBuffers in pBackBuffer speichern

		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateRenderTargetView(		// Erstellt eine View auf Backbuffer, können nur Views an Pipeline binden
															// View wird später per OMSetRenderTarget an OutputMerger-Stage gebunden
			m_pBackBuffer.Get(),							// Wert des Pointers, also Adresse des IF-Pointers
			0,												// Description für View, NULL wenn 1. Param kein Mipmap hat
			&m_pTarRenderView));							// RenderTargetView ist an m_pBackBuffer gebunden
		return hr;
	}//END-FUNC
	/// <summary>
	/// Erstellt einen VertexBuffer-Speicher. Mit CreateBuffer wird Speicher auf GPU allocated und mit SetVertexBuffers wird es auf GPU kopiert
	/// Referenz zu meinem VertexStruct, dass folgende Daten beinhaltet:
	///	uStride: Größe eines Elementes im VertexArray
	///	uOffset: Offset im Array
	///	VertexBufferDesc: Description die benötigt wird um ein VertexBuffer auf GPU zu erstellen
	///	aVertices:	Punkte für das Zeichnen der 2 Dreiecke (Quad)
	///	VertexSubResourceData: beinhaltet die Daten (aVertices) 
	///	</summary>
	HRESULT ClsD3D11::CreateVertexBuffer()
	{
		VertexBufferContent VBufContent;						// Punkte
		HRESULT hr = NULL;										// Rückgabewert von ApiFkt. für Errorhandling
		VBufContent.VertexBufferDesc.BindFlags
			= D3D11_BIND_VERTEX_BUFFER;							// Buffer soll für Vertexe verwendet werden, Vertex Buffer in IA-Stage
		VBufContent.VertexBufferDesc.Usage
			= D3D11_USAGE_DEFAULT;								// Default: GPU read and write
		VBufContent.VertexBufferDesc.ByteWidth
			= sizeof(VBufContent.aVertices);					// Gesamte Größe von aVertices oder: sizeof(Vertex*4) 80
		VBufContent.VertexSubResData.pSysMem
			= VBufContent.aVertices;							// Zuweisung der Daten in SubResourceTyp
		VBufContent.uStride = sizeof(Vertex);					// Größe pro Element: 20
		VBufContent.uOffset = 0;								// Offset, wenn er von Element zu Element springt (um z.B. erste 4 Bits zu ignorieren)

		// auf GPU allocaten
		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateBuffer(
			&VBufContent.VertexBufferDesc,
			&VBufContent.VertexSubResData,
			&VBufContent.pVertexBuffer));
		// auf GPU kopieren
		m_pD3dContext->IASetVertexBuffers(
			0,
			1,
			VBufContent.pVertexBuffer.GetAddressOf(),
			&VBufContent.uStride,
			&VBufContent.uOffset);

		return hr;
	}//END-FUNC
	/// <summary>
	/// Erstellt einen IndexBuffer. Dieser Index bezieht sich auf den VertexBuffer. 
	/// Um ein Viereck mit 2 Dreiecken darzustellen, benötigt man an wenigen Stellen 2 Punkte die eine identische Position haben.
	/// um Speicher zu sparen wird dort nicht jedesmal ein neuer Vertex definiert, sondern bezieht sich auf ein Index.
	/// Damit wird jeder Vertex nur einmal angelegt, kann aber mehrmals per Index genutzt werden
	/// Referenz zu meinem IndexStruct, dass folgende Daten beinhaltet:
	///	IndexBufferDesc: Description die benötigt wird um ein IndexBuffer auf GPU zu erstellen
	///	iIndices:	Indices für die Vertices 1= Vertex 1 in aVertices
	///	IndexSubResData: beinhaltet die Daten (iIndices) </summary>
	HRESULT ClsD3D11::CreateIndexBuffer()
	{
		HRESULT hr = NULL;									// Rückgabewert von ApiFkt. für Errorhandling
		m_myIndexBuffer.myIndexBufferDesc.BindFlags			// Buffer soll als IndexBuffer genutzt werde-> IndexBufferFlag setzen
			= D3D11_BIND_INDEX_BUFFER;						// Buffer soll für VertexIndices verwendet werden, IndexBuffer in IA-Stage
		m_myIndexBuffer.myIndexBufferDesc.Usage
			= D3D11_USAGE_DEFAULT;							// Default: GPU read and write
		m_myIndexBuffer.myIndexBufferDesc.ByteWidth
			= sizeof(m_myIndexBuffer.iIndices);				// Gesamte Größe von iIndices oder: sizeof(iIndices*6)= sizeof(short)*6 = 16*6
		m_myIndexBuffer.myIndexBufferDesc.CPUAccessFlags = 0;
		m_myIndexBuffer.myIndexBufferDesc.MiscFlags = 0;
		m_myIndexBuffer.myIndexSubResData.pSysMem
			= m_myIndexBuffer.iIndices;						// Zuweisung der Daten in SubResourceTyp
		// Reservierung des Speichers auf GPU
		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateBuffer(	// Erstellung des IndexBuffer auf GPU
			&m_myIndexBuffer.myIndexBufferDesc,				// Bufferbeschreibung (als IndexBuffer)
			&m_myIndexBuffer.myIndexSubResData,				// Datencontainer	
			&m_myIndexBuffer.pIndexBuffer));				// IF-Pointer zum IndexBufferObj

		m_pD3dContext->IASetIndexBuffer(					// COPY GPU zu GPU
			m_myIndexBuffer.pIndexBuffer.Get(),				// setzt alle notwendigen Daten, die dann beim StageDurchgang abgefragt werden
			DXGI_FORMAT_R16_UINT, 0);						// Format der Elemente (short integer)

		return hr;
	}//END-FUNC
	/// <summary>
	/// Erstellung des ConstantBuffers: 
	/// - Erstellen einer Matrix
	/// - Matrix als Subresource speichern
	/// - als Buffer in GPU allocaten
	/// Hier werden nicht die Daten auf GPU kopiert, weil dies ja pro Frame passiert
	/// und vorher von CPU noch manipuliert wird - Schreibrechte
	/// Referenz zu meinem ConstantBuffer, dass folgende Daten beinhaltet:
	///	fAngle: Winkel der pro Frame geändert wird (in Matrix)
	///	ConstantBuffer:	Array aus structs was Matrizen enthält (hier nur ein Element, da nur eine Matrix)
	///	ConstantSubResData: beinhaltet die Daten (Matrizen) </summary>
	HRESULT ClsD3D11::CreateConstantBuffer()
	{
		HRESULT hr = NULL;

		D3D11_BUFFER_DESC ConstantBufferDesc = {};					// Description für einen Buffer
		D3D11_SUBRESOURCE_DATA ConstantSubResData = {};				// Data Container	

		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// Buffer als ConstantBuffer def.
		ConstantBufferDesc.ByteWidth = sizeof(m_myConstantBuffer.myConstantBuffer);	// Array von Matritzen, haben nur ein Element: 4x4xfloat = 4*4*4 = 64 Bytes
		ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// wird pro Frame geändert, muss für CPU gelockt werden
		ConstantBufferDesc.MiscFlags = 0;
		ConstantBufferDesc.StructureByteStride = 0;					// Größe pro Element, wird nicht benötigt
		ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;				// Dynamic: CPU: write; GPU: read
		ConstantSubResData.pSysMem = &m_myConstantBuffer.myConstantBuffer;			// Daten in Datencontainer speichern
		// Constant Buffer allocaten
		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateBuffer(
			&ConstantBufferDesc,
			&ConstantSubResData,
			&m_myConstantBuffer.pConstantBuffer));

		return hr;
	}//END-FUNC
	/// <summary>
	/// Calls all private Create Buffer methods: Vertex-, Index- and ConstantBuffers
	/// </summary>
	/// <returns>HResult</returns>
	HRESULT ClsD3D11::CreateBuffers()
	{
		HRESULT hr = NULL;
		HR_RETURN_ON_ERR(hr, CreateVertexBuffer());
		HR_RETURN_ON_ERR(hr, CreateIndexBuffer());
		HR_RETURN_ON_ERR(hr, CreateConstantBuffer());
	}
	/// <summary>
	/// Modifikation der TranformationsMatrix: 
	/// - Drehung der Vertices/Koordinaten auf GPU mit Hilfe einer Transformationsmatrix
	/// - wird pro Frame auf GPU geladen
	/// - Schneller als alle Vertices pro Frame neu hochzuladen:
	///		- weniger Daten von CPU zu GPU
	///     - Manipulation wird auf GPU ausgeführt. GPU ist wesentlich schneller für solche Operationen.
	/// - CPU benötigt auf ConstantBuffer AccessRights: 
	///		- Daten (Matrizen) werden nach jedem Frame gemappt für GPU gesperrt und CPU kopiert Daten
	///	    - GPU liest Daten danach aus und kopiert sie auf VRAM
	/// m_ConstantBuffer:
	///	- fAngle: Winkel der pro Frame geändert wird (in Matrix)
	///	- MyConstantBuffer:	Array aus structs was Matrizen enthält (hier nur ein Element/Matrix)
	///	- pConstantBuffer: Pointer auf Daten
	/// </summary>
	HRESULT ClsD3D11::SetConstantBuffer()
	{
		HRESULT hr = NULL;
		m_myConstantBuffer.fAngle = m_oTimer.Peek();			// Winkeländerung pro Frame: Zeitdifferenz zum letzten Durchgang
		D3D11_MAPPED_SUBRESOURCE mappedRes;						// Datencontainer

		// 4x4 Matrix
		// 1. x-Koord
		// 2. y-Koord
		// 3. z-Koord: keine Änderung, d.h. Einheitswerte
		// 4. für Mondifikation: keine Änderung, d.h. Einheitswerte
		// 4x2 Matrix geht nicht für Berechnung -> Zur Berechnung muss Anzahl Zeile = Anzahl Spalte -> 4x4Matrix
		m_myConstantBuffer.myConstantBuffer =
		{
			{
				std::cos(m_myConstantBuffer.fAngle), std::sin(m_myConstantBuffer.fAngle), 0.0f, 0.0f,
				-std::sin(m_myConstantBuffer.fAngle), std::cos(m_myConstantBuffer.fAngle), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			}
		};
		// Locken des Buffers, sodass CPU drauf schreiben kann (gpu muss mit lesen warten)
		HR_RETURN_ON_ERR(hr, m_pD3dContext->Map(
			m_myConstantBuffer.pConstantBuffer.Get(),			// Welcher Buffer soll gelockt werden
			0,													// Index für Subresource (können mehrere im Array sein)
			D3D11_MAP_WRITE_DISCARD,							// gelockt fürs schreiben, alter Inhalt wird nicht mehr definiert sein (nicht gemerkt)
			0,													// Flag setzen falls nicht gelockt werden kann und was CPU derweil tun soll
			&mappedRes));										// Datencontainer des Buffers

		ConstantBuffer* dataPtr = (ConstantBuffer*)mappedRes.pData; // Zeiger auf Daten im Datencontainer 
		memcpy(dataPtr, &m_myConstantBuffer.myConstantBuffer, sizeof(ConstantBuffer)); // neue Daten reinschreiben
		m_pD3dContext->Unmap(m_myConstantBuffer.pConstantBuffer.Get(), 0); // Buffer wieder unlocken (CPU ist fertig)
		m_pD3dContext->VSSetConstantBuffers(0, 1, m_myConstantBuffer.pConstantBuffer.GetAddressOf()); // Manipulierten Buffer direkt in VS-Stage setzen

		return hr;
	}//END-FUNC
	/// <summary>
	/// Erstellt einen Sampler, der die Texture auf das Quad (2 Dreiecke) mappen kann.
	/// Benötigt für jedes Pixel Koordinaten auf Texture um die richtige Farbe darzustellen.
	/// Die UV-TextureCoord werden von InputAssamblerStage (Input von Vertexdaten: Coord, UV-Maps,Farbe) bis in den PixelshaderStage übertragen
	/// Siehe Input PixelShader
	/// Bei der Erstellung eines Samplers hat dieser eine feste Position in der GPU.
	/// Sampler kann somit im Pixelshader wie folgt angesprochen werden: 
	/// register(s0) s = Sampler, 0 = Index im PixelShader.hlsl
	/// </summary>
	HRESULT ClsD3D11::CreateAndSetSampler()
	{
		HRESULT hr = NULL;
		D3D11_SAMPLER_DESC samplerDesc = {};
		ComPtr<ID3D11SamplerState> samplerState;				// Sampler IF-Pointer

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	// punktuelles Mapping (pro Punkt eine Koordinate),wird nicht interpoliert/berechnet
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;	// Freiräume bekommen eine Farbe, es wird die Texture nicht gespiegelt etc.
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.BorderColor[0] = 1.0f;						// Farbe bei Freiräumen
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;	// Verhalten von alten zu neuen Daten, z.B. bei Desktopdupl nur mappen wo Änderung

		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateSamplerState(&samplerDesc, &samplerState));	// Sampler auf GPU allocaten
		m_pD3dContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());			// Sampler setzen (COPY von GPU zu GPU)

		return hr;
	}//END-FUNC

	/// <summary>
	/// - Creates a D3D11Texture2D depending on the EnumType: Mapping, D2D1Surface, SubResource
	///	- Mapping:	
	///		- D3D11_USAGE_DYNAMIC (usage) and D3D11_CPU_ACCESS_WRITE (cpuflags)
	///		- CPU can lock the Subresource for coping BitmapData
	///	- D2D1Surface:
	///  	- D3D11_USAGE_DEFAULT (usage) and 0 (cpuflags)
	///		- init a Surface above the D3D11Texture and writes the BitmapData
	///		- no binded SubResource to the D3D11Texture2D needed 
	///		- needs a D2D1Surface
	/// - SubResource:
	///		- D3D11_USAGE_DEFAULT (usage) and 0 (cpuflags)
	///		- CPU must wait until the Subresource is free to write
	/// </summary>
	/// <param name="MyBitBltTexture">Struct mit ImgDaten: Breite, Höhe, Bytes pro Pixel, Pointer zu den Pixeldaten</param>
	HRESULT ClsD3D11::CreateD3D11Texture()
	{
		HRESULT hr = NULL;

		D3D11_TEXTURE2D_DESC TextureDesc = {};							// TextureDescription für TextureObj
		D3D11_SUBRESOURCE_DATA SubResData = {};							// Container für Texture Daten

		UINT& uiHeightDest = m_pFrameData->uiHeightDest;
		UINT& uiWidthDest = m_pFrameData->uiWidthDest;
		UINT& uiBpp = m_pFrameData->uiBpp;
		unsigned char* pData = m_pFrameData->pData;

		TextureDesc.Width = uiWidthDest;								// Width
		TextureDesc.Height = uiHeightDest;								// Height
		TextureDesc.MipLevels = 1;										// MipLvl 0/1 = aus
		TextureDesc.ArraySize = 1;										// ArraySize der Subresource, da nur 1. Adresse des 1. Elements übergeben wird = 1
		TextureDesc.Format = PFORMAT;									// Bildformat: Desktopdupl. ist immer BGRA
		TextureDesc.SampleDesc.Count = 1;

		switch (m_myCpyMethod)
		{
			case CopyMethod::D2D1Surface:

				TextureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;		// wird an VertexShader-Stage gebunden
				TextureDesc.Usage = D3D11_USAGE_DEFAULT;				// Default: Read/Write when is not locked by GPU
				TextureDesc.CPUAccessFlags = 0;							// keine CPU-Rights notwendig
				HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateTexture2D(&TextureDesc, NULL, &m_pD3D11Texture));
				HR_RETURN_ON_ERR(hr, InitD2D1());
				break;
			case CopyMethod::DesktopDupl:

				TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		// wird an VertexShader-Stage gebunden
				TextureDesc.Usage = D3D11_USAGE_DYNAMIC;				// Default: Read/Write when is not locked by GPU
				TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// Write Access for the CPU
				SubResData.pSysMem = (const void*)pData;				// Daten in Container speichern
				SubResData.SysMemPitch = uiWidthDest * uiBpp;			// Eine Pixelreihe als Width*4: 1 Pixel = 4 Bytes
				SubResData.SysMemSlicePitch = 0;						// nur wichtig bei 3D-Texturen
				HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateTexture2D(&TextureDesc, NULL, &m_pD3D11Texture));
				HR_RETURN_ON_ERR(hr, InitDesktopDuplication());
				break;
			case CopyMethod::Mapping:

				TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		// wird an VertexShader-Stage gebunden
				TextureDesc.Usage = D3D11_USAGE_DYNAMIC;				// CPU can lock SubResource of the Texture for Write
				TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// Write Access for the CPU
				
				SubResData.pSysMem = (const void*)pData;				// Daten in Container speichern
				SubResData.SysMemPitch = uiWidthDest * uiBpp;			// Eine Pixelreihe als Width*4: 1 Pixel = 4 Bytes
				SubResData.SysMemSlicePitch = 0;						// nur wichtig bei 3D-Texturen
				HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateTexture2D(&TextureDesc, &SubResData, &m_pD3D11Texture));
				break;

			case CopyMethod::SubResource:

				TextureDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;		// wird an VertexShader-Stage gebunden
				TextureDesc.Usage = D3D11_USAGE_DEFAULT;				// Default: Read/Write when is not locked by GPU
				TextureDesc.CPUAccessFlags = 0;							// keine CPU-Rights notwendig
				
				SubResData.pSysMem = (const void*)pData;				// Daten in Container speichern
				SubResData.SysMemPitch = uiWidthDest * uiBpp;			// Eine Pixelreihe als Width*4: 1 Pixel = 4 Bytes
				SubResData.SysMemSlicePitch = 0;						// nur wichtig bei 3D-Texturen
				HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateTexture2D(&TextureDesc, &SubResData, &m_pD3D11Texture));
				break;
		}//END-Switch
		return hr;
	}//END-FUNC
	/// <summary>
	/// - MonitorObjekt mit DxgiOutput bzw. DxgiOutput1
	/// - DxgiOutput1 hat DuplicationFunction
	/// - m_pDeskDupl: DesktopObj womit Frames ausgelesen werden könne
	/// - m_MyOutputDuplDesc: Properties des Monitorabbildes
	/// </summary>
	/// <returns></returns>
	HRESULT ClsD3D11::InitDesktopDuplication()
	{
		HRESULT hr = NULL;
		ComPtr<IDXGIOutput> pDxgiOutput;
		ComPtr<IDXGIOutput1> pDxgiOutput1;
		
		HR_RETURN_ON_ERR(hr, m_pdxgiAdapter->EnumOutputs(m_uiPickedMonitor, &pDxgiOutput));
		//if (FAILED(hResult))
		//	return 0;
		HR_RETURN_ON_ERR(hr, pDxgiOutput->QueryInterface(IID_PPV_ARGS(&pDxgiOutput1)));// Es gibt 2 IF die ein DxgiOutput erbt/implementiert. 
																// Bei der Erstellung eines Objs wird es über eine CreateFunktion erstellt, per Polymorphism:
																// Je nachdem ob Obj per Polym über Output1 oder Output 2 erstellt wird überschreibt es unterschiedliche Funktionen
																// QuaryInterface ist eine art CrossCast: IFOutput1 und IFOutput2 erben von der selben Klasse,
																// def. aber unterschiedliche leere virtuelle Funktionen
																// Interface
																// |     |
																// IF1   IF2
																// |      |
																// HiddenObjClass(MyIFPtr)
																// Create: Interface*ptr; MyIFPtr = new IF2; 
		if (!pDxgiOutput1)
			return 0;

		HR_RETURN_ON_ERR(hr, pDxgiOutput1->DuplicateOutput(		// Creates a complete Duplication of a Output/Monitor 
			m_pD3dDevice.Get(),
			&m_pDeskDupl));										// pDeskDupl beinhalted Zugriff auf Frames etc.
		//m_pDeskDupl->GetDesc(&m_MyOutputDuplDesc);				// Description des Monitors wie Auflösung etc.

		return hr;
	}
	/// <summary>
	/// - Erstellung einer m_pShaderView. 
	/// - View mit m_pD3D11Texture wird an PixelShader-Stage gebunden 
	/// - bei Ausführung von PixelShader an OutputMerger übergeben
	/// - OutputMerger beinhaltet Buffer der SwapChain
	/// ->in der Pipeline (Pixelshader zum OutputMerger) wird meine m_pD3D11Texture an m_pBackBuffer übergeben
	/// </summary>
	HRESULT ClsD3D11::CreateShaderView()
	{
		HRESULT hr = NULL;

		HR_RETURN_ON_ERR(hr,
			m_pD3dDevice->CreateShaderResourceView(
				m_pD3D11Texture.Get(),
				nullptr,
				&m_pShaderView));
		m_pD3dContext->PSSetShaderResources(0, 1, m_pShaderView.GetAddressOf()); // beinhaltet unsere Texture, welche unser ImgTexture oder DesktopDuplTexture beinhaltet

		return hr;
	}
	/// <summary>
	/// Compiliert ein ShaderProgramm für die GPU
	/// m_pD3dDevice->CreateVertexShader: allocaten des Speichers auf GPU für ShaderProgramm
	/// m_pD3dContext->VSSetShader: kopiert compiliertes ShaderProgramm auf GPU in die VertexShader-Engine
	/// Es wird ein IF-Pointer zum erstellten VertexShader-Obj erstellt: pVertexShader
	/// </summary>
	/// <param name="vsBlob">Referenz zum leeren ComPtr für ShaderProgrammDaten, wo sie gespeichert werden sollen</param>
	/// <returns></returns>
	HRESULT ClsD3D11::CreateVetexShader()
	{
		ComPtr<ID3D11VertexShader> pVertexShader;			// IF-Pointer zum VertexShaderProgramm auf GPU
		ComPtr<ID3DBlob> pShaderCompileErrorsBlob;			// IF-Pointer zum Error
		HRESULT hr = NULL;
		HR_RETURN_ON_ERR(hr, D3DCompileFromFile(
			L"VertexShader1.hlsl",							// ShaderProgrammCode
			nullptr,
			nullptr,
			"main",											// EntryPoint zur MainFunktion im ShaderProgramm
			"vs_5_0",										// vordefinierte Strings die angeben was wir compilieren
			0,												// Flags
			0,												// Flags
			&m_pVertexShaderBlob,							// IF-Pointer, hat Zugriff auf compilierte Programmdaten
			&pShaderCompileErrorsBlob));						// IF-Pointer, hat Zugriff auf CompilerErrors

		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateVertexShader(
			m_pVertexShaderBlob->GetBufferPointer(),		// IF-Pointer zu den compilierten Programmdaten 
			m_pVertexShaderBlob->GetBufferSize(),			// Größe der Daten
			nullptr,
			&pVertexShader));								// IF-Pointer, indirekter Zugriff auf ContainerObj für VertexShaderProgramm 
		m_pD3dContext->VSSetShader(pVertexShader.Get(), nullptr, 0); // COPY intern von GPU(creationplace) zu GPU(runplace)

		return hr;
	}//END-FUNC
	/// <summary>
	/// Compiliert ein ShaderProgramm für die GPU
	/// m_pD3dDevice->CreatePixelShader: allocaten des Speichers auf GPU für ShaderProgramm
	/// m_pD3dContext->PSSetShader: kopiert compiliertes ShaderProgramm auf GPU in die VertexShader-Engine
	/// Es wird ein IF-Pointer zum erstellten VertexShader-Obj erstellt: pPixelShader
	/// </summary>
	/// <returns></returns>
	HRESULT ClsD3D11::CreatePixelShader()
	{
		HRESULT hr = NULL;
		ComPtr<ID3D11PixelShader> pPixelShader;					// IF-Pointer zum VertexShaderProgramm auf GPU
		ComPtr<ID3DBlob> pShaderCompileErrorsBlob;				// IF-Pointer zum Error
		//ComPtr<ID3DBlob> psBlob;

		HR_RETURN_ON_ERR(hr, D3DCompileFromFile(
			L"PixelShader.hlsl",
			nullptr,
			nullptr,
			"main",												// EntryPoint zur MainFunktion im ShaderProgramm
			"ps_5_0",											// vordefinierte Strings die angeben was wir compilieren
			0,													// Flags
			0,													// Flags
			&m_pPixelShaderBlob,											// IF-Pointer, hat Zugriff auf compilierte Programmdaten
			&pShaderCompileErrorsBlob));							// IF-Pointer, hat Zugriff auf CompilerErrors
		/*if (FAILED(hResult))									// falls was schiefgeht, CompilerErrors ausgeben
		{
			const char* errorString = NULL;
			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				errorString = "Could not compile shader; file not found";
			else if (pShaderCompileErrorsBlob) {
				errorString = (const char*)pShaderCompileErrorsBlob->GetBufferPointer();
				pShaderCompileErrorsBlob->Release();
			}
			MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
			return 0;
		}*/

		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreatePixelShader(
			m_pPixelShaderBlob->GetBufferPointer(),							// IF-Pointer zu den compilierten Programmdaten 
			m_pPixelShaderBlob->GetBufferSize(),							// Größe der Daten
			nullptr,
			&pPixelShader));										// IF-Pointer, indirekter Zugriff auf ContainerObj für VertexShaderProgramm 
		//assert(SUCCEEDED(hResult));
		m_pD3dContext->PSSetShader(pPixelShader.Get(), nullptr, 0); // COPY intern von GPU(creationplace) zu GPU(runplace)

		return hr;
	}//END-FUNC
	/// <summary>
	/// Create and set Pixel- and VertexShader
	/// </summary>
	/// <returns></returns>
	HRESULT ClsD3D11::CreateAndSetShader()
	{
		HRESULT hr = NULL;
		HR_RETURN_ON_ERR(hr, CreateVetexShader());
		HR_RETURN_ON_ERR(hr, CreatePixelShader());

		return hr;
	}
	/// <summary>
	/// Setzt ParamterTypen zischen InputAssamblerStage und VertexShaderStage fest
	/// Allocated diese entsprechend
	/// - Legt Typ und Größe der Inputparamter fest
	/// - Übergibt nicht selbst die InputParamter: erst beim Pipeline-Durchgang (ShaderProgrammaufrufe) aufgerufen werden
	/// - m_pVertexShaderBlob: compiliertes ShaderProgramm, beinhaltet Pointer und Größe</summary>
	HRESULT ClsD3D11::CreateInputLayout()
	{
		HRESULT hr = NULL;
		ComPtr<ID3D11InputLayout> pInputLayout;					// IF-Pointer zum Layout

		const D3D11_INPUT_ELEMENT_DESC InputElementDesc[] =
		{
			{"POS",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEX",0,DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"COL",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
			// Nachdem die Werte den Vertexshader verlassen, müssen sie normalized sein
			// mit UNORM sind sie sogar schon normalized wenn sie in den Vertexshader reinkommen
			// ABER: die Bitgröße muss  beachtet werden, wenn die Color aus 4 floats besteht (je 32 bit), dann darf ich kein DXGI_FORMAT_R16G16B16A16_UNORM verweden
			// unsigned char ist 8 bit groß. Pro Farbe RGBA/BGRA. Ergibt 4*8Bit = 32Bit: DXGI_FORMAT_R8G8B8A8_UNORM

		};
		// Schnittstelle allocaten (Parameterliste) zwischen InputAssambler und VertexShader auf GPU
		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateInputLayout(
			InputElementDesc,									// Beschreibung der Inputparameter für ShaderProgramm
			ARRAYSIZE(InputElementDesc),						// Anzahl der Elemente
			m_pVertexShaderBlob->GetBufferPointer(),			// Pointer zum kompilierten Shaderprogramm auf GPU
			m_pVertexShaderBlob->GetBufferSize(),				// Größe des ShaderProgramms
			&pInputLayout));										// IF-Pointer zum LayoutObj
		//assert(SUCCEEDED(hResult));
		m_pD3dContext->IASetInputLayout(pInputLayout.Get());	// Schnittstelle zwischen IAssambler und VertexShader festlegen

		return hr;
	}//END-FUNC
	/// <summary>
	/// Kopiervorgang der PixelDaten innerhalb der Loop.
	/// Je nach CopyMethod wird die Methode ausgewählt.
	/// Kopieren auf die D3D11Texture zum anzeigen und kopieren auf CPU-Seite für SinkWriter zum aufnhemen eines Videos
	/// </summary>
	/// <returns></returns>
	HRESULT ClsD3D11::BitBltDataToRT()
	{
		HRESULT hr = NULL;
		switch (m_myCpyMethod)
		{
			case CopyMethod::Mapping:
				HR_RETURN_ON_ERR(hr, BitBltDataToD3D11SubResource());
				break;

			case CopyMethod::D2D1Surface:
				HR_RETURN_ON_ERR(hr, BitBltDataToD2D1Resource());
				break;

			case CopyMethod::SubResource:
				HR_RETURN_ON_ERR(hr, UpdateMySubResource());
				break;
			case CopyMethod::DesktopDupl:
				HR_RETURN_ON_ERR(hr, GetDesktopDuplByGPU());
				break;
		}
		return hr;
	}//END-FUNC
	/// <summary>
	/// Erstellt aus Bilddaten eine Subresource. 
	/// Input: BitBltData: Width, Height, Bpp, PixelData 
	/// MySubResource ist an m_pD3D11Texture gebunden:
	/// Locks the Subresource: CPU can now Write ImageData to the Subresource
	///		- Texture that includes the Subresource must be have the following settings:
	///			-D3D11_USAGE_DYNAMIC
	///			-CPU_ACCESS_RIGHT
	/// m_pD3D11Texture ist an m_pShaderView gebunden
	/// m_pShaderView ist als Input an PixelShader gebunden
	/// </summary>
	/// <param name="MyBitBltData"></param>
	HRESULT ClsD3D11::BitBltDataToD3D11SubResource()
	{
		HRESULT hr = NULL;
		int iErr = 0;
		UINT& uiPixelDataSize = m_pFrameData->uiPixelDataSize;
		unsigned char* pData = m_pFrameData->pData;
		

		ZeroMemory(&m_mySubResD3D11Texture, sizeof(D3D11_MAPPED_SUBRESOURCE));
		m_pD3dContext->Map(
			m_pD3D11Texture.Get(),
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&m_mySubResD3D11Texture);

		
		memcpy_s(m_mySubResD3D11Texture.pData, uiPixelDataSize, pData, uiPixelDataSize);			// Pixeldaten in SubResource von D3d11Texture kopieren
		m_pD3dContext->Unmap(m_pD3D11Texture.Get(), 0);
		return hr;
	}
	/// <summary>
	/// Updates the D2d1Rendertarget with the current BitmapData
	/// Enum: - D2D1Surface
	/// Info: - m_pD3D11Texture must be created with TextureDescription: D3D11_USAGE_DEFAULT and CPUflags = 0
	///		  - Bitmap binded to the D2D1RenderTarget
	///		  - D2D1RenderTarget binded to the  dxgiSurface
	///		  - DxgiSurface binded to the m_pD3D11Texture
	///	      - m_pD3D11Texture to the ShaderView (which is drawn in the PixelShader)
	///		  - Pipeline: PixelShader to the OutputMerger (m_pD3D11Texture to the m_pBackBuffer in SwapChain)
	/// </summary>
	/// <returns></returns>
	HRESULT ClsD3D11::BitBltDataToD2D1Resource()
	{
		HRESULT hr = NULL;
		HR_RETURN_ON_ERR(hr, m_pClsD2D1->UpdateD2D1RenderTarget());

		return hr;
	}//END-FUNC
	
	/// <summary>
	/// Updates the m_pD3D11Texture with the current BitmapData pData
	/// Enum: - SubResource
	/// Info: - m_pD3D11Texture must be created with TextureDescription: D3D11_USAGE_DEFAULT and CPUflags = 0
	/// CalledBy: BitBltDataToRT()
	/// </summary>
	HRESULT ClsD3D11::UpdateMySubResource()
	{
		UINT& uiHeightDest = m_pFrameData->uiHeightDest;
		UINT& uiWidthDest = m_pFrameData->uiWidthDest;
		UINT& uiBpp = m_pFrameData->uiBpp;
		unsigned char* pData = m_pFrameData->pData;

		UINT uiPitch = uiWidthDest * uiBpp;
		D3D11_BOX destRegion;


		destRegion.left = 0;
		destRegion.right = uiWidthDest;
		destRegion.top = 0;
		destRegion.bottom = uiHeightDest;
		destRegion.front = 0;
		destRegion.back = 1;

		m_pD3dContext->UpdateSubresource(m_pD3D11Texture.Get(), 0, &destRegion, pData, uiPitch, 0);

		return S_OK;
	}//END-FUNC
	/// <summary>
	/// - Fragt nächstes Frame von m_pDeskDupl bzw. des Monitorabbildes ab
	/// - Frame muss released werden bevor nächstes Frame abgerufen werden kann
	/// - Überschreibt m_pD3D11Texture mit akt. Frame
	/// - m_pD3D11Texture ist an m_pShaderView gebunden
	/// - m_pShaderView ist als Input an PixelShader gebunden
	/// - PixelShader sendet alles an OutputMergerStage
	/// - OutputMergerStage beinhaltet Buffer der Swapchain
	/// CalledBy: BitBltDataToRT()
	/// </summary>
	HRESULT ClsD3D11::GetDesktopDuplByGPU()
	{
		HRESULT hr = NULL;
		HR_RETURN_ON_ERR(hr, m_pDeskDupl->AcquireNextFrame(
			500,												// Framegültigkeit in ms. Muss noch beim nächsten Durchlauf verfügbar sein
			&m_MyFrameInfo,										// MouseCurserInformationen 
																// letzte Aktualisierung des DesktopFrames seitens DWM
			&m_pDxgiDesktopResource));							// Container für ImgDaten
		// LowLvl (Hardwarenah) zu HighLvl
		
		HR_RETURN_ON_ERR(hr, m_pDxgiDesktopResource->QueryInterface(
			IID_PPV_ARGS(&m_pD3dAcquiredDesktopImage)));

		
		// Überschreibung meiner D3D11Texture mit DesktopD3D11Texture
		m_pD3dContext->CopyResource(m_pD3D11Texture.Get(), m_pD3dAcquiredDesktopImage.Get());
		//D3D11_TEXTURE2D_DESC myDesc;
		//m_pD3dAcquiredDesktopImage->GetDesc(&myDesc);
		HR_RETURN_ON_ERR(hr, DesktopDuplToRAM()); // für Sinkwriter
		//m_pD3dContext->UpdateSubresource(m_pD3D11Texture.Get(), 0, &destRegion, pData, uiPitch, 0);
		HR_RETURN_ON_ERR(hr, m_pDeskDupl->ReleaseFrame());				// Frame wieder freigeben, Abfrage beendet
		
		return hr;
	}//END-FUNC
	/// <summary>
	/// - Kopiert Bilddaten der DesktopDupl in eine Texture - von GPU zu GPU.
	/// - Ziel ist eine Texture auf GPU, wo CPU Kopierrechte hat.
	/// - Bilddaten werden dann von VRAM zu RAM kopiert:
	///   - GPU zu GPU. Dann GPU zu CPU
	///   - wird für Sinkwriter benötigt zur Erstellung des Videos
	///   - wird nicht zur Anzeige benötigt (geht alles über GPU)
	/// CalledBy: GetDesktopDuplByGPU()
	/// </summary>
	HRESULT ClsD3D11::DesktopDuplToRAM()
	{
		HRESULT hr = NULL;
		UINT& uiPixelDataSize = m_pFrameData->uiPixelDataSize;
		unsigned char* pData = m_pFrameData->pData;

		m_pD3dContext->CopyResource(m_pD3D11TextureCPUAccess.Get(), m_pD3dAcquiredDesktopImage.Get());// Kopieren der Bilddaten auf Texture mit CPU Read Access

		// Subressource, erlaubt Zugriff auf Bilddaten
		memset(&m_mySubResD3D11Texture, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		// Locken damit CPU lesen kann
		HR_RETURN_ON_ERR(hr, m_pD3dContext->Map(
			m_pD3D11TextureCPUAccess.Get(),
			0,
			D3D11_MAP_READ,
			0,
			&m_mySubResD3D11Texture));
		m_mySubResD3D11Texture.RowPitch;
		// Daten in RAM speichern (CPU-Side)
		memcpy_s(pData, uiPixelDataSize, m_mySubResD3D11Texture.pData, uiPixelDataSize);			// Pixeldaten in SubResource von D3d11Texture kopieren
																											// Lock freigeben. GPU hat wieder vollen Zugriff
		m_pD3dContext->Unmap(m_pD3D11TextureCPUAccess.Get(), 0);
		return hr;
	}//END-FUNC
	/// <summary>
	/// Preparing D3D11Texture for copying Data to the CPU-Side.
	/// Used if CopyMethod::DeskDupl
	/// CalledBy: PreparePresentation
	/// </summary>
	/// <returns></returns>
	HRESULT ClsD3D11::PrepareDesktopDuplToRAM()
	{
		HRESULT hr = NULL;
		UINT& uiHeightDest = m_pFrameData->uiHeightDest;
		UINT& uiWidthDest = m_pFrameData->uiWidthDest;
		
		m_myTextureDescCPUAccess.Width = uiWidthDest;								// Width
		m_myTextureDescCPUAccess.Height = uiHeightDest;								// Height
		m_myTextureDescCPUAccess.MipLevels = 1;										// MipLvl 0/1 = aus
		m_myTextureDescCPUAccess.ArraySize = 1;										// ArraySize der Subresource, da nur 1. Adresse des 1. Elements übergeben wird = 1
		m_myTextureDescCPUAccess.Format = PFORMAT;									// Bildformat: Desktopdupl. ist immer BGRA
		m_myTextureDescCPUAccess.SampleDesc.Count = 1;
		m_myTextureDescCPUAccess.Usage = D3D11_USAGE_STAGING;						// Staging: GPU hat vollen Zugriff. CPU kann lesen
		m_myTextureDescCPUAccess.CPUAccessFlags = D3D11_CPU_ACCESS_READ;				// Read Access by CPU
		HR_RETURN_ON_ERR(hr, m_pD3dDevice->CreateTexture2D(&m_myTextureDescCPUAccess, NULL, &m_pD3D11TextureCPUAccess));

		return hr;
	}//END-FUNC
	/// <summary>
	/// Festlegen wie das fertige Bild im Frontbuffer angezeigt werden soll
	/// Passt Bild für das ZielFenster an: MyViewport
	/// Stages: Vertexshader->Rasterizer->Pixelshader->OutputMerger
	/// Setzt Topology für mein 3D-Umgebung. Diese ist nur ein 2D-Quad das aufs voll Fenster gestreckt wird
	///		- 2 Dreiecke die ein Quad ergeben.
	/// Legt das RenderTarget fest. Meine RenderView die an den BackBuffer gebunden ist. 
	/// BackBuffer ist an m_pD3D11Texture gebunden. Texture wird über DeskDupl oder per Mapping von GDI geupdatet
	/// </summary>
	HRESULT ClsD3D11::PreparePresentation()
	{
		HRESULT hr = S_OK;
		

		// Bindet die View. Damit später der Rasterizer weiss wieviel max. dargestellt werden soll
		D3D11_VIEWPORT myViewport = { 0.0f, 0.0f, (FLOAT)(m_myClientRect.right - m_myClientRect.left), (FLOAT)(m_myClientRect.bottom - m_myClientRect.top), 0.0f, 1.0f };
		m_pD3dContext->RSSetViewports(1, &myViewport);			// Die Area wo Rasterizer zeichnet, genau definieren
		/* Erstellt das sichtbare Bild aus:
		* Vertexshader: Dargestellte Form (Übergabe an PixelShader)
		* PixelShader: Sampler mit UV-Mapping. Mapped Texture auf dargestellte Form. (Übergabe an OutputMerger)
		* Backbuffer/TargetRenderView: Hintergrund/ClearColor (direktes setzen im OutputMerger)
		* DephstencilView: Bei 3D, welche Ebene ist weiter vorn und sichtbar und welche nicht (direktes setzen im OutputMerger)*/
		
		/**********Input Assembler**********/
		m_pD3dContext->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			// Legt interpretation der eingehenden Daten/Vertices fest: 
															// Punkte, Linie, Linien, Dreicke, zusammenhängende Dreiecke etc.
		m_pD3dContext->OMSetRenderTargets(1, m_pTarRenderView.GetAddressOf(), 0);

		if (m_myCpyMethod == CopyMethod::DesktopDupl)
			HR_RETURN_ON_ERR(hr,PrepareDesktopDuplToRAM());

		return hr;
	}//END-FUNC
	/// <summary>
	/// Durchlauf der D3D11-Stages mit .Draw() bzw. .DrawIndexed() und Benutzen der vorh. def. Variablen wie Backbuffer/Texture
	/// BackBuffer wird zum Frontbuffer und Bild wird somit angezeigt
	/// </summary>
	/// <returns></returns>
	HRESULT ClsD3D11::PresentTexture()
	{
		HRESULT hr = NULL;
		m_pD3dContext->DrawIndexed(							// verwenden Indices fürs zeichnen
			(UINT)std::size(m_myIndexBuffer.iIndices),			// sizeof(iIndices/iIndices[0]); 12/2 = 6; 12Byte/2Byte (2Byte/16Bit = short int)
			0,												// StartIndex
			0												// IndexAddition: bevor mit jedem Index im VertexArray gesucht wird, wird zum Index 1 addiert
		);

		HR_RETURN_ON_ERR(hr, m_pDxgiSwapChain->Present(
			1,												/* Wie das fertige Bild in den FrontBuffer geladen wird:
															Für die BitBlockÜbertragung DXGI_SWAP_EFFECT_DISCARD oder DXGI_SWAP_EFFECT_SEQUENTIAL
															0:	keine Sync. Bild wird in den FrontBuffer geladen sobald es fertig ist
															1-4: Anzahl der VBlanks die abgewartet werden bis neues Bild in den FrontBuffer geladen wird
															*/
			0												// Flags: z.B. Tearing erlauben, neuladen eines Bildes, ohne VBlanks zu berücksichtigen
		));

		return hr;
	}//END-FUNC
	/// <summary>
	/// Destructor
	/// </summary>
	ClsD3D11::~ClsD3D11()
	{
		if (m_pClsD2D1)
		{
			delete m_pClsD2D1;
			m_pClsD2D1 = nullptr;
		}
	}//END-CONS
}//END-NAMESPACE D3D