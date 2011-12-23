#include "testApp.h"
//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
float g_pDepthHist[MAX_DEPTH];
XnRGB24Pixel* g_pTexMap = NULL;
unsigned int g_nTexMapX = 0;
unsigned int g_nTexMapY = 0;

unsigned int g_nViewState = DEFAULT_DISPLAY_MODE;

Context g_context;
ScriptNode g_scriptNode;
DepthGenerator g_depth;
DepthMetaData g_depthMD;
//--------------------------------------------------------------
void testApp::setup(){
	
	XnStatus rc;
	
	EnumerationErrors errors;
	rc = g_context.InitFromXmlFile(SAMPLE_XML_PATH, g_scriptNode, &errors);
	if (rc == XN_STATUS_NO_NODE_PRESENT)
	{
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		printf("%s\n", strError);
		return ;
	}
	else if (rc != XN_STATUS_OK)
	{
		printf("Open failed: %s\n", xnGetStatusString(rc));
		return;
	}
	
	rc = g_context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_depth);
	if (rc != XN_STATUS_OK)
	{
		printf("No depth node exists! Check your XML.");
		return;
	}
	
	g_depth.GetMetaData(g_depthMD);
	
	// Texture map init
	g_nTexMapX = (((unsigned short)(g_depthMD.FullXRes()-1) / 512) + 1) * 512;
	g_nTexMapY = (((unsigned short)(g_depthMD.FullYRes()-1) / 512) + 1) * 512;
	g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));
	std::cout << " w:" << g_depthMD.FullXRes() << " h:" << g_depthMD.FullYRes() << std::endl;
	pixels = (unsigned char*)malloc(640*480*3*sizeof(unsigned char));
	tex.allocate(640, 480, GL_RGB);
}

//--------------------------------------------------------------
void testApp::update(){
	XnStatus rc = XN_STATUS_OK;
	
	// Read a new frame
	rc = g_context.WaitAnyUpdateAll();
	if (rc != XN_STATUS_OK)
	{
		printf("Read failed: %s\n", xnGetStatusString(rc));
		return;
	}
	
	g_depth.GetMetaData(g_depthMD);
	//g_image.GetMetaData(g_imageMD);
	
	const XnDepthPixel* pDepth = g_depthMD.Data();
	
	// Calculate the accumulative histogram (the yellow display...)
	xnOSMemSet(g_pDepthHist, 0, MAX_DEPTH*sizeof(float));
	
	unsigned int nNumberOfPoints = 0;
	for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
	{
		for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth)
		{
			if (*pDepth != 0)
			{
				g_pDepthHist[*pDepth]++;
				nNumberOfPoints++;
			}
		}
	}
	for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (int nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}
	
	xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));
	
	// check if we need to draw depth frame to texture
	const XnDepthPixel* pDepthRow = g_depthMD.Data();
	XnRGB24Pixel* pTexRow = g_pTexMap + g_depthMD.YOffset() * g_nTexMapX;
	
	for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
	{
		const XnDepthPixel* pDepth = pDepthRow;
		XnRGB24Pixel* pTex = pTexRow + g_depthMD.XOffset();
		
		for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth, ++pTex)
		{
			int idx = (x + y * g_depthMD.XRes()) * 3;
			if (*pDepth != 0)
			{
				int nHistValue = g_pDepthHist[*pDepth];
				
				pixels[idx] = nHistValue;
				pixels[idx+1] = nHistValue;
				pixels[idx+2] = nHistValue;
			}
			else
			{
				pixels[idx] = 0;
				pixels[idx+1] = 0;
				pixels[idx+2] = 0;
			}
		}
		
		pDepthRow += g_depthMD.XRes();
		pTexRow += g_nTexMapX;
	}
	
	tex.loadData(pixels, 640, 480, GL_RGB);
}

//--------------------------------------------------------------
void testApp::draw(){
	tex.draw(0,0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}