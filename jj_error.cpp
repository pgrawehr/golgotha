#include "pch.h"

/*=========================================================================*
*                                                                         *
*  Name:       JError.cpp                                                 *
*  Purpose:    Standard DX Error Processing                               *
*                                                                         *
*  Copyright (c) 1998 Drago                                               *
*                                                                         *
*  Author:     Jan Jasinski (TRACE)                                       *
*                                                                         *
*  Remarks:    This file can be used under windows ONLY.                  *
*=========================================================================*/
#define DIRECTINPUT_VERSION 0x0500
#include <mmsystem.h>
#include <d3d9.h>
#include <dsound.h>
//#include "DSOUND.H"
#include <d3dcommon.h>
#include <dinput.h>
//#include <DxErr.h>
#include "dxfile.h"
#include "dxerr9.h"
#include "jj_error.h"

//—————————————————————————————————
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif // _DEBUG
//—————————————————————————————————
/*———————————————————————————————————————————————————————————————————————————*
*  GLobals for LOGs                                                         *
*———————————————————————————————————————————————————————————————————————————*/
BOOL g_bLogFile             = FALSE;
int iDebugLogFile          = -1; // Not opened or error
/*———————————————————————————————————————————————————————————————————————————*
*  DIRECT DRAW ERRORS                                                       *
*———————————————————————————————————————————————————————————————————————————*/
char * dxerr_unknown = "DDERR_??? (UNKNOWN)";
static ERRLIST elDDErrors[] = {
	{DDERR_ALREADYINITIALIZED,          "DDERR_ALREADYINITIALIZED"},            //   5
	{DDERR_CANNOTATTACHSURFACE,         "DDERR_CANNOTATTACHSURFACE"},           //  10
	{DDERR_CANNOTDETACHSURFACE,         "DDERR_CANNOTDETACHSURFACE"},           //  20
	{DDERR_CURRENTLYNOTAVAIL,           "DDERR_CURRENTLYNOTAVAIL"},             //  40
	{DDERR_EXCEPTION,                   "DDERR_EXCEPTION"},                     //  55
	{DDERR_GENERIC,                     "DDERR_GENERIC"},                       //    E_FAIL
	{DDERR_HEIGHTALIGN,                 "DDERR_HEIGHTALIGN"},                   //  90
	{DDERR_INCOMPATIBLEPRIMARY,         "DDERR_INCOMPATIBLEPRIMARY"},           //  95
	{DDERR_INVALIDCAPS,                 "DDERR_INVALIDCAPS"},                   // 100
	{DDERR_INVALIDCLIPLIST,             "DDERR_INVALIDCLIPLIST"},               // 110
	{DDERR_INVALIDMODE,                 "DDERR_INVALIDMODE"},                   // 120
	{DDERR_INVALIDOBJECT,               "DDERR_INVALIDOBJECT"},                 // 130
	{DDERR_INVALIDPARAMS,               "DDERR_INVALIDPARAMS"},                 //    E_INVALIDARG
	{DDERR_INVALIDPIXELFORMAT,          "DDERR_INVALIDPIXELFORMAT"},            // 145
	{DDERR_INVALIDRECT,                 "DDERR_INVALIDRECT"},                   // 150
	{DDERR_LOCKEDSURFACES,              "DDERR_LOCKEDSURFACES"},                // 160
	{DDERR_NO3D,                        "DDERR_NO3D"},                          // 170
	{DDERR_NOALPHAHW,                   "DDERR_NOALPHAHW"},                     // 180
	{DDERR_NOSTEREOHARDWARE,            "DDERR_NOSTEREOHARDWARE"},              // 181 ( DirectX 7.0 )
	{DDERR_NOSURFACELEFT,               "DDERR_NOSURFACELEFT"},                 // 182 ( DirectX 7.0 )
	{DDERR_NOCLIPLIST,                  "DDERR_NOCLIPLIST"},                    // 205
	{DDERR_NOCOLORCONVHW,               "DDERR_NOCOLORCONVHW"},                 // 210
	{DDERR_NOCOOPERATIVELEVELSET,       "DDERR_NOCOOPERATIVELEVELSET"},         // 212
	{DDERR_NOCOLORKEY,                  "DDERR_NOCOLORKEY"},                    // 215
	{DDERR_NOCOLORKEYHW,                "DDERR_NOCOLORKEYHW"},                  // 220
	{DDERR_NODIRECTDRAWSUPPORT,         "DDERR_NODIRECTDRAWSUPPORT"},           // 222
	{DDERR_NOEXCLUSIVEMODE,             "DDERR_NOEXCLUSIVEMODE"},               // 225
	{DDERR_NOFLIPHW,                    "DDERR_NOFLIPHW"},                      // 230
	{DDERR_NOGDI,                       "DDERR_NOGDI"},                         // 240
	{DDERR_NOMIRRORHW,                  "DDERR_NOMIRRORHW"},                    // 250
	{DDERR_NOTFOUND,                    "DDERR_NOTFOUND"},                      // 255
	{DDERR_NOOVERLAYHW,                 "DDERR_NOOVERLAYHW"},                   // 260
	{DDERR_OVERLAPPINGRECTS,            "DDERR_OVERLAPPINGRECTS"},              // 270 ( DirectX 6.0 )
	{DDERR_NORASTEROPHW,                "DDERR_NORASTEROPHW"},                  // 280
	{DDERR_NOROTATIONHW,                "DDERR_NOROTATIONHW"},                  // 290
	{DDERR_NOSTRETCHHW,                 "DDERR_NOSTRETCHHW"},                   // 310
	{DDERR_NOT4BITCOLOR,                "DDERR_NOT4BITCOLOR"},                  // 316
	{DDERR_NOT4BITCOLORINDEX,           "DDERR_NOT4BITCOLORINDEX"},             // 317
	{DDERR_NOT8BITCOLOR,                "DDERR_NOT8BITCOLOR"},                  // 320
	{DDERR_NOTEXTUREHW,                 "DDERR_NOTEXTUREHW"},                   // 330
	{DDERR_NOVSYNCHW,                   "DDERR_NOVSYNCHW"},                     // 335
	{DDERR_NOZBUFFERHW,                 "DDERR_NOZBUFFERHW"},                   // 340
	{DDERR_NOZOVERLAYHW,                "DDERR_NOZOVERLAYHW"},                  // 350
	{DDERR_OUTOFCAPS,                   "DDERR_OUTOFCAPS"},                     // 360
	{DDERR_OUTOFMEMORY,                 "DDERR_OUTOFMEMORY"},                   //    E_OUTOFMEMORY
	{DDERR_OUTOFVIDEOMEMORY,            "DDERR_OUTOFVIDEOMEMORY"},              // 380
	{DDERR_OVERLAYCANTCLIP,             "DDERR_OVERLAYCANTCLIP"},               // 382
	{DDERR_OVERLAYCOLORKEYONLYONEACTIVE,"DDERR_OVERLAYCOLORKEYONLYONEACTIVE"},  // 384
	{DDERR_PALETTEBUSY,                 "DDERR_PALETTEBUSY"},                   // 387
	{DDERR_COLORKEYNOTSET,              "DDERR_COLORKEYNOTSET"},                // 400
	{DDERR_SURFACEALREADYATTACHED,      "DDERR_SURFACEALREADYATTACHED"},        // 410
	{DDERR_SURFACEALREADYDEPENDENT,     "DDERR_SURFACEALREADYDEPENDENT"},       // 420
	{DDERR_SURFACEBUSY,                 "DDERR_SURFACEBUSY"},                   // 430
	{DDERR_CANTLOCKSURFACE,             "DDERR_CANTLOCKSURFACE"},               // 435
	{DDERR_SURFACEISOBSCURED,           "DDERR_SURFACEISOBSCURED"},             // 440
	{DDERR_SURFACELOST,                 "DDERR_SURFACELOST"},                   // 450
	{DDERR_SURFACENOTATTACHED,          "DDERR_SURFACENOTATTACHED"},            // 460
	{DDERR_TOOBIGHEIGHT,                "DDERR_TOOBIGHEIGHT"},                  // 470
	{DDERR_TOOBIGSIZE,                  "DDERR_TOOBIGSIZE"},                    // 480
	{DDERR_TOOBIGWIDTH,                 "DDERR_TOOBIGWIDTH"},                   // 490
	{DDERR_UNSUPPORTED,                 "DDERR_UNSUPPORTED"},                   //    E_NOTIMPL
	{DDERR_UNSUPPORTEDFORMAT,           "DDERR_UNSUPPORTEDFORMAT"},             // 510
	{DDERR_UNSUPPORTEDMASK,             "DDERR_UNSUPPORTEDMASK"},               // 520
	{DDERR_INVALIDSTREAM,               "DDERR_INVALIDSTREAM"},                 // 521 ( DirectX 6.0 )
	{DDERR_VERTICALBLANKINPROGRESS,     "DDERR_VERTICALBLANKINPROGRESS"},       // 537
	{DDERR_WASSTILLDRAWING,             "DDERR_WASSTILLDRAWING"},               // 540
	{DDERR_DDSCAPSCOMPLEXREQUIRED,      "DDERR_DDSCAPSCOMPLEXREQUIRED"},        // 542 ( DirectX 7.0 )
	{DDERR_XALIGN,                      "DDERR_XALIGN"},                        // 560
	{DDERR_INVALIDDIRECTDRAWGUID,       "DDERR_INVALIDDIRECTDRAWGUID"},         // 561
	{DDERR_DIRECTDRAWALREADYCREATED,    "DDERR_DIRECTDRAWALREADYCREATED"},      // 562
	{DDERR_NODIRECTDRAWHW,              "DDERR_NODIRECTDRAWHW"},                // 563
	{DDERR_PRIMARYSURFACEALREADYEXISTS, "DDERR_PRIMARYSURFACEALREADYEXISTS"},   // 564
	{DDERR_NOEMULATION,                 "DDERR_NOEMULATION"},                   // 565
	{DDERR_REGIONTOOSMALL,              "DDERR_REGIONTOOSMALL"},                // 566
	{DDERR_CLIPPERISUSINGHWND,          "DDERR_CLIPPERISUSINGHWND"},            // 567
	{DDERR_NOCLIPPERATTACHED,           "DDERR_NOCLIPPERATTACHED"},             // 568
	{DDERR_NOHWND,                      "DDERR_NOHWND"},                        // 569
	{DDERR_HWNDSUBCLASSED,              "DDERR_HWNDSUBCLASSED"},                // 570
	{DDERR_HWNDALREADYSET,              "DDERR_HWNDALREADYSET"},                // 571
	{DDERR_NOPALETTEATTACHED,           "DDERR_NOPALETTEATTACHED"},             // 572
	{DDERR_NOPALETTEHW,                 "DDERR_NOPALETTEHW"},                   // 573
	{DDERR_BLTFASTCANTCLIP,             "DDERR_BLTFASTCANTCLIP"},               // 574
	{DDERR_NOBLTHW,                     "DDERR_NOBLTHW"},                       // 575
	{DDERR_NODDROPSHW,                  "DDERR_NODDROPSHW"},                    // 576
	{DDERR_OVERLAYNOTVISIBLE,           "DDERR_OVERLAYNOTVISIBLE"},             // 577
	{DDERR_NOOVERLAYDEST,               "DDERR_NOOVERLAYDEST"},                 // 578
	{DDERR_INVALIDPOSITION,             "DDERR_INVALIDPOSITION"},               // 579
	{DDERR_NOTAOVERLAYSURFACE,          "DDERR_NOTAOVERLAYSURFACE"},            // 580
	{DDERR_EXCLUSIVEMODEALREADYSET,     "DDERR_EXCLUSIVEMODEALREADYSET"},       // 581
	{DDERR_NOTFLIPPABLE,                "DDERR_NOTFLIPPABLE"},                  // 582
	{DDERR_CANTDUPLICATE,               "DDERR_CANTDUPLICATE"},                 // 583
	{DDERR_NOTLOCKED,                   "DDERR_NOTLOCKED"},                     // 584
	{DDERR_CANTCREATEDC,                "DDERR_CANTCREATEDC"},                  // 585
	{DDERR_NODC,                        "DDERR_NODC"},                          // 586
	{DDERR_WRONGMODE,                   "DDERR_WRONGMODE"},                     // 587
	{DDERR_IMPLICITLYCREATED,           "DDERR_IMPLICITLYCREATED"},             // 588
	{DDERR_NOTPALETTIZED,               "DDERR_NOTPALETTIZED"},                 // 589
	{DDERR_UNSUPPORTEDMODE,             "DDERR_UNSUPPORTEDMODE"},               // 590
	{DDERR_NOMIPMAPHW,                  "DDERR_NOMIPMAPHW"},                    // 591
	{DDERR_INVALIDSURFACETYPE,          "DDERR_INVALIDSURFACETYPE"},            // 592
	{DDERR_NOOPTIMIZEHW,                "DDERR_NOOPTIMIZEHW"},                  // 600
	{DDERR_NOTLOADED,                   "DDERR_NOTLOADED"},                     // 601
	{DDERR_NOFOCUSWINDOW,               "DDERR_NOFOCUSWINDOW"},                 // 602 ( DirectX 6.0 )
	{DDERR_NOTONMIPMAPSUBLEVEL,         "DDERR_NOTONMIPMAPSUBLEVEL"},           // 603 ( DirectX 7.0 )
	{DDERR_DCALREADYCREATED,            "DDERR_DCALREADYCREATED"},              // 620
	{DDERR_NONONLOCALVIDMEM,            "DDERR_NONONLOCALVIDMEM"},              // 630
	{DDERR_CANTPAGELOCK,                "DDERR_CANTPAGELOCK"},                  // 640
	{DDERR_CANTPAGEUNLOCK,              "DDERR_CANTPAGEUNLOCK"},                // 660
	{DDERR_NOTPAGELOCKED,               "DDERR_NOTPAGELOCKED"},                 // 680
	{DDERR_MOREDATA,                    "DDERR_MOREDATA"},                      // 690
	{DDERR_EXPIRED,                     "DDERR_EXPIRED" },                      // 691 ( DirectX 7.0 )
	{DDERR_TESTFINISHED,                "DDERR_TESTFINISHED"},                  // 692 ( DirectX 7.0 )
	{DDERR_NEWMODE,                     "DDERR_NEWMODE"},                       // 693 ( DirectX 7.0 )
	{DDERR_D3DNOTINITIALIZED,           "DDERR_D3DNOTINITIALIZED"},             // 694 ( DirectX 7.0 )
	{DDERR_VIDEONOTACTIVE,              "DDERR_VIDEONOTACTIVE"},                // 695
	{DDERR_NOMONITORINFORMATION,        "DDERR_NOMONITORINFORMATION"},          // 696 ( DirectX 7.0 )
	{DDERR_NODRIVERSUPPORT,             "DDERR_NODRIVERSUPPORT"},               // 697 ( DirectX 7.0 )
	{DDERR_DEVICEDOESNTOWNSURFACE,      "DDERR_DEVICEDOESNTOWNSURFACE"},        // 699
	{DDERR_NOTINITIALIZED,              "DDERR_NOTINITIALIZED"}                 //    CO_E_NOTINITIALIZED
}; // end of "elDDErrors[]"

const int ERRLISTLEN =  (sizeof(elDDErrors)/sizeof(ERRLIST))    ;

/*———————————————————————————————————————————————————————————————————————————*
*  DIRECT 3D ERRORS                                                         *
*———————————————————————————————————————————————————————————————————————————*/

static ERRLIST elD3DErrors[] = {
/*	{D3DERR_BADMAJORVERSION,            "D3DERR_BADMAJORVERSION"},              // 700
	{D3DERR_BADMINORVERSION,            "D3DERR_BADMINORVERSION"},              // 701
	{D3DERR_INVALID_DEVICE,             "D3DERR_INVALID_DEVICE"},               // 705
	{D3DERR_INITFAILED,                 "D3DERR_INITFAILED"},                   // 706
	{D3DERR_DEVICEAGGREGATED,           "D3DERR_DEVICEAGGREGATED "},            // 707
	{D3DERR_EXECUTE_CREATE_FAILED,      "D3DERR_EXECUTE_CREATE_FAILED"},        // 710
	{D3DERR_EXECUTE_DESTROY_FAILED,     "D3DERR_EXECUTE_DESTROY_FAILED"},       // 711
	{D3DERR_EXECUTE_LOCK_FAILED,        "D3DERR_EXECUTE_LOCK_FAILED"},          // 712
	{D3DERR_EXECUTE_UNLOCK_FAILED,      "D3DERR_EXECUTE_UNLOCK_FAILED"},        // 713
	{D3DERR_EXECUTE_LOCKED,             "D3DERR_EXECUTE_LOCKED"},               // 714
	{D3DERR_EXECUTE_NOT_LOCKED,         "D3DERR_EXECUTE_NOT_LOCKED"},           // 715
	{D3DERR_EXECUTE_FAILED,             "D3DERR_EXECUTE_FAILED"},               // 716
	{D3DERR_EXECUTE_CLIPPED_FAILED,     "D3DERR_EXECUTE_CLIPPED_FAILED"},       // 717
	{D3DERR_TEXTURE_NO_SUPPORT,         "D3DERR_TEXTURE_NO_SUPPORT"},           // 720
	{D3DERR_TEXTURE_CREATE_FAILED,      "D3DERR_TEXTURE_CREATE_FAILED"},        // 721
	{D3DERR_TEXTURE_DESTROY_FAILED,     "D3DERR_TEXTURE_DESTROY_FAILED"},       // 722
	{D3DERR_TEXTURE_LOCK_FAILED,        "D3DERR_TEXTURE_LOCK_FAILED"},          // 723
	{D3DERR_TEXTURE_UNLOCK_FAILED,      "D3DERR_TEXTURE_UNLOCK_FAILED"},        // 724
	{D3DERR_TEXTURE_LOAD_FAILED,        "D3DERR_TEXTURE_LOAD_FAILED"},          // 725
	{D3DERR_TEXTURE_SWAP_FAILED,        "D3DERR_TEXTURE_SWAP_FAILED"},          // 726
	{D3DERR_TEXTURE_LOCKED,             "D3DERR_TEXTURE_LOCKED"},               // 727
	{D3DERR_TEXTURE_NOT_LOCKED,         "D3DERR_TEXTURE_NOT_LOCKED"},           // 728
	{D3DERR_TEXTURE_GETSURF_FAILED,     "D3DERR_TEXTURE_GETSURF_FAILED"},       // 729
	{D3DERR_MATRIX_CREATE_FAILED,       "D3DERR_MATRIX_CREATE_FAILED"},         // 730
	{D3DERR_MATRIX_DESTROY_FAILED,      "D3DERR_MATRIX_DESTROY_FAILED"},        // 731
	{D3DERR_MATRIX_SETDATA_FAILED,      "D3DERR_MATRIX_SETDATA_FAILED"},        // 732
	{D3DERR_MATRIX_GETDATA_FAILED,      "D3DERR_MATRIX_GETDATA_FAILED"},        // 733
	{D3DERR_SETVIEWPORTDATA_FAILED,     "D3DERR_SETVIEWPORTDATA_FAILED"},       // 734
	{D3DERR_INVALIDCURRENTVIEWPORT,     "D3DERR_INVALIDCURRENTVIEWPORT"},       // 735
	{D3DERR_INVALIDPRIMITIVETYPE,       "D3DERR_INVALIDPRIMITIVETYPE"},         // 736
	{D3DERR_INVALIDVERTEXTYPE,          "D3DERR_INVALIDVERTEXTYPE"},            // 737
	{D3DERR_TEXTURE_BADSIZE,            "D3DERR_TEXTURE_BADSIZE"},              // 738
	{D3DERR_INVALIDRAMPTEXTURE,         "D3DERR_INVALIDRAMPTEXTURE"},           // 739
	{D3DERR_MATERIAL_CREATE_FAILED,     "D3DERR_MATERIAL_CREATE_FAILED"},       // 740
	{D3DERR_MATERIAL_DESTROY_FAILED,    "D3DERR_MATERIAL_DESTROY_FAILED"},      // 741
	{D3DERR_MATERIAL_SETDATA_FAILED,    "D3DERR_MATERIAL_SETDATA_FAILED"},      // 742
	{D3DERR_MATERIAL_GETDATA_FAILED,    "D3DERR_MATERIAL_GETDATA_FAILED"},      // 743
	{D3DERR_INVALIDPALETTE,             "D3DERR_INVALIDPALETTE"},               // 744
	{D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY,   "D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY"},     // 745
	{D3DERR_ZBUFF_NEEDS_VIDEOMEMORY,    "D3DERR_ZBUFF_NEEDS_VIDEOMEMORY"},      // 746
	{D3DERR_SURFACENOTINVIDMEM,         "D3DERR_SURFACENOTINVIDMEM"},           // 747
	{D3DERR_LIGHT_SET_FAILED,           "D3DERR_LIGHT_SET_FAILED"},             // 750
	{D3DERR_LIGHTHASVIEWPORT,           "D3DERR_LIGHTHASVIEWPORT"},             // 751
	{D3DERR_LIGHTNOTINTHISVIEWPORT,     "D3DERR_LIGHTNOTINTHISVIEWPORT"},       // 752
	{D3DERR_SCENE_IN_SCENE,             "D3DERR_SCENE_IN_SCENE"},               // 760
	{D3DERR_SCENE_NOT_IN_SCENE,         "D3DERR_SCENE_NOT_IN_SCENE"},           // 761
	{D3DERR_SCENE_BEGIN_FAILED,         "D3DERR_SCENE_BEGIN_FAILED"},           // 762
	{D3DERR_SCENE_END_FAILED,           "D3DERR_SCENE_END_FAILED"},             // 763
	{D3DERR_INBEGIN,                    "D3DERR_INBEGIN"},                      // 770
	{D3DERR_NOTINBEGIN,                 "D3DERR_NOTINBEGIN"},                   // 771
	{D3DERR_NOVIEWPORTS,                "D3DERR_NOVIEWPORTS"},                  // 772
	{D3DERR_VIEWPORTDATANOTSET,         "D3DERR_VIEWPORTDATANOTSET"},           // 773
	{D3DERR_VIEWPORTHASNODEVICE,        "D3DERR_VIEWPORTHASNODEVICE"},          // 774
	{D3DERR_NOCURRENTVIEWPORT,          "D3DERR_NOCURRENTVIEWPORT"},            // 775
	{D3DERR_INVALIDVERTEXFORMAT,        "D3DERR_INVALIDVERTEXFORMAT"},          //2048 ( DirectX 6.0 )
	{D3DERR_COLORKEYATTACHED,           "D3DERR_COLORKEYATTACHED"},             //2050 ( DirectX 6.0 )
	{D3DERR_VERTEXBUFFEROPTIMIZED,      "D3DERR_VERTEXBUFFEROPTIMIZED"},        //2060 ( DirectX 6.0 )
	{D3DERR_VBUF_CREATE_FAILED,         "D3DERR_VBUF_CREATE_FAILED"},           //2061 ( DirectX 6.0 )
	{D3DERR_VERTEXBUFFERLOCKED,         "D3DERR_VERTEXBUFFERLOCKED"},           //2062 ( DirectX 6.0 )
	{D3DERR_VERTEXBUFFERUNLOCKFAILED,   "D3DERR_VERTEXBUFFERUNLOCKFAILED"},     //2063 ( DirectX 7.0 )
	{D3DERR_ZBUFFER_NOTPRESENT,         "D3DERR_ZBUFFER_NOTPRESENT"},           //2070 ( DirectX 6.0 )
	{D3DERR_STENCILBUFFER_NOTPRESENT,   "D3DERR_STENCILBUFFER_NOTPRESENT"},     //2071 ( DirectX 6.0 ) */
	{D3DERR_WRONGTEXTUREFORMAT,         "D3DERR_WRONGTEXTUREFORMAT"},           //2072 ( DirectX 6.0 )
	{D3DERR_UNSUPPORTEDCOLOROPERATION,  "D3DERR_UNSUPPORTEDCOLOROPERATION"},    //2073 ( DirectX 6.0 )
	{D3DERR_UNSUPPORTEDCOLORARG,        "D3DERR_UNSUPPORTEDCOLORARG"},          //2074 ( DirectX 6.0 )
	{D3DERR_UNSUPPORTEDALPHAOPERATION,  "D3DERR_UNSUPPORTEDALPHAOPERATION"},    //2075 ( DirectX 6.0 )
	{D3DERR_UNSUPPORTEDALPHAARG,        "D3DERR_UNSUPPORTEDALPHAARG"},          //2076 ( DirectX 6.0 )
	{D3DERR_TOOMANYOPERATIONS,          "D3DERR_TOOMANYOPERATIONS"},            //2077 ( DirectX 6.0 )
	{D3DERR_CONFLICTINGTEXTUREFILTER,   "D3DERR_CONFLICTINGTEXTUREFILTER"},     //2078 ( DirectX 6.0 )
	{D3DERR_UNSUPPORTEDFACTORVALUE,     "D3DERR_UNSUPPORTEDFACTORVALUE"},       //2079 ( DirectX 6.0 )
	{D3DERR_CONFLICTINGRENDERSTATE,     "D3DERR_CONFLICTINGRENDERSTATE"},       //2081 ( DirectX 6.0 )
	{D3DERR_UNSUPPORTEDTEXTUREFILTER,   "D3DERR_UNSUPPORTEDTEXTUREFILTER"},     //2082 ( DirectX 6.0 )
/*	{D3DERR_TOOMANYPRIMITIVES,          "D3DERR_TOOMANYPRIMITIVES"},            //2083 ( DirectX 6.0 )
	{D3DERR_INVALIDMATRIX,              "D3DERR_INVALIDMATRIX"},                //2084 ( DirectX 6.0 )
	{D3DERR_TOOMANYVERTICES,            "D3DERR_TOOMANYVERTICES"},              //2085 ( DirectX 6.0 ) */
	{D3DERR_CONFLICTINGTEXTUREPALETTE,  "D3DERR_CONFLICTINGTEXTUREPALETTE"},    //2086 ( DirectX 6.0 )
/*	{D3DERR_INVALIDSTATEBLOCK,          "D3DERR_INVALIDSTATEBLOCK"},            //2100 ( DirectX 7.0 )
	{D3DERR_INBEGINSTATEBLOCK,          "D3DERR_INBEGINSTATEBLOCK"},            //2101 ( DirectX 7.0 )
	{D3DERR_NOTINBEGINSTATEBLOCK,       "D3DERR_NOTINBEGINSTATEBLOCK"}          //2102 ( DirectX 7.0 )*/
}; // end of "elD3DErrors[]"

const int ERRD3DLISTLEN =  (sizeof(elD3DErrors)/sizeof(ERRLIST))    ;


/*———————————————————————————————————————————————————————————————————————————*
*  DIRECT SOUND ERRORS                                                      *
*———————————————————————————————————————————————————————————————————————————*/

static ERRLIST elDSErrors[] = {
	{DS_NO_VIRTUALIZATION,              "DS_NO_VIRTUALIZATION"},                //0,10
	{DSERR_ALLOCATED,                   "DSERR_ALLOCATED"},                     //  10               ( DirectX 7.0)
	{DSERR_CONTROLUNAVAIL,              "DSERR_CONTROLUNAVAIL"},                //  30
	{DSERR_INVALIDPARAM,                "DSERR_INVALIDPARAM"},                  //    E_INVALIDARG
	{DSERR_INVALIDCALL,                 "DSERR_INVALIDCALL"},                   //  50
	{DSERR_GENERIC,                     "DSERR_GENERIC"},                       //    E_FAIL
	{DSERR_PRIOLEVELNEEDED,             "DSERR_PRIOLEVELNEEDED"},               //  70
	{DSERR_OUTOFMEMORY,                 "DSERR_OUTOFMEMORY"},                   //    E_OUTOFMEMORY
	{DSERR_BADFORMAT,                   "DSERR_BADFORMAT"},                     // 100
	{DSERR_UNSUPPORTED,                 "DSERR_UNSUPPORTED"},                   //    E_NOTIMPL
	{DSERR_NODRIVER,                    "DSERR_NODRIVER"},                      // 120
	{DSERR_ALREADYINITIALIZED,          "DSERR_ALREADYINITIALIZED"},            // 130
	{DSERR_NOAGGREGATION,               "DSERR_NOAGGREGATION"},                 //    CLASS_E_NOAGGREGATION
	{DSERR_BUFFERLOST,                  "DSERR_BUFFERLOST"},                    // 150
	{DSERR_OTHERAPPHASPRIO,             "DSERR_OTHERAPPHASPRIO"},               // 160
	{DSERR_UNINITIALIZED,               "DSERR_UNINITIALIZED"},                 // 170
	{DSERR_NOINTERFACE,                 "DSERR_NOINTERFACE"},                   //    E_NOINTERFACE
	{DSERR_ACCESSDENIED,                "DSERR_ACCESSDENIED"}                   //    E_ACCESSDENIED ( DirectX 7.0)
}; // end of "elDSErrors[]"

const int ERRDSLISTLEN =  (sizeof(elDSErrors)/sizeof(ERRLIST));

/*———————————————————————————————————————————————————————————————————————————*
*  DIRECT INPUT ERRORS                                                      *
*———————————————————————————————————————————————————————————————————————————*/
static ERRLIST elDIErrors[] = {
	{DI_NOTATTACHED,                    "DI_NOTATTACHED"},                      //    S_FALSE
	{DI_BUFFEROVERFLOW,                 "DI_BUFFEROVERFLOW"},                   //    S_FALSE
	{DI_PROPNOEFFECT,                   "DI_PROPNOEFFECT"},                     //    S_FALSE
	{DI_NOEFFECT,                       "DI_NOEFFECT"},                         //    S_FALSE
	{DI_POLLEDDEVICE,                   "DI_POLLEDDEVICE"},                     //0002
	{DI_DOWNLOADSKIPPED,                "DI_DOWNLOADSKIPPED"},                  //0003
	{DI_EFFECTRESTARTED,                "DI_EFFECTRESTARTED"},                  //0004
	{DI_TRUNCATED,                      "DI_TRUNCATED"},                        //0008
	{DI_TRUNCATEDANDRESTARTED,          "DI_TRUNCATEDANDRESTARTED"},            //000C
	{DIERR_OLDDIRECTINPUTVERSION,       "DIERR_OLDDIRECTINPUTVERSION"},         //    ERROR_OLD_WIN_VERSION
	{DIERR_BETADIRECTINPUTVERSION,      "DIERR_BETADIRECTINPUTVERSION"},        //    ERROR_RMODE_APP
	{DIERR_BADDRIVERVER,                "DIERR_BADDRIVERVER"},                  //    ERROR_BAD_DRIVER_LEVEL
	{DIERR_DEVICENOTREG,                "DIERR_DEVICENOTREG"},                  //    REGDB_E_CLASSNOTREG
	{DIERR_NOTFOUND,                    "DIERR_NOTFOUND"},                      //    ERROR_FILE_NOT_FOUND
	{DIERR_OBJECTNOTFOUND,              "DIERR_OBJECTNOTFOUND"},                //    ERROR_FILE_NOT_FOUND
	{DIERR_INVALIDPARAM,                "DIERR_INVALIDPARAM"},                  //    E_INVALIDARG
	{DIERR_NOINTERFACE,                 "DIERR_NOINTERFACE"},                   //    E_NOINTERFACE
	{DIERR_GENERIC,                     "DIERR_GENERIC"},                       //    E_FAIL
	{DIERR_OUTOFMEMORY,                 "DIERR_OUTOFMEMORY"},                   //    E_OUTOFMEMORY
	{DIERR_UNSUPPORTED,                 "DIERR_UNSUPPORTED"},                   //    E_NOTIMPL
	{DIERR_NOTINITIALIZED,              "DIERR_NOTINITIALIZED"},                //    ERROR_NOT_READY
	{DIERR_ALREADYINITIALIZED,          "DIERR_ALREADYINITIALIZED"},            //    ERROR_ALREADY_INITIALIZED
	{DIERR_NOAGGREGATION,               "DIERR_NOAGGREGATION"},                 //    CLASS_E_NOAGGREGATION
	{DIERR_OTHERAPPHASPRIO,             "DIERR_OTHERAPPHASPRIO"},               //    E_ACCESSDENIED
	{DIERR_INPUTLOST,                   "DIERR_INPUTLOST"},                     //    ERROR_READ_FAULT
	{DIERR_ACQUIRED,                    "DIERR_ACQUIRED"},                      //    ERROR_BUSY
	{DIERR_NOTACQUIRED,                 "DIERR_NOTACQUIRE"},                    //    ERROR_INVALID_ACCESS
	{DIERR_READONLY,                    "DIERR_READONLY"},                      //    E_ACCESSDENIED
	{DIERR_HANDLEEXISTS,                "DIERR_HANDLEEXISTS"},                  //    E_ACCESSDENIED
	{E_PENDING,                         "E_PENDING"},                           //8000000A
	{DIERR_INSUFFICIENTPRIVS,           "DIERR_INSUFFICIENTPRIVS"},             //80040200
	{DIERR_DEVICEFULL,                  "DIERR_DEVICEFULL"},                    //80040201
	{DIERR_MOREDATA,                    "DIERR_MOREDATA"},                      //80040202
	{DIERR_NOTDOWNLOADED,               "DIERR_NOTDOWNLOADED"},                 //80040203
	{DIERR_HASEFFECTS,                  "DIERR_HASEFFECTS"},                    //80040204
	{DIERR_NOTEXCLUSIVEACQUIRED,        "DIERR_NOTEXCLUSIVEACQUIRED"},          //80040205
	{DIERR_INCOMPLETEEFFECT,            "DIERR_INCOMPLETEEFFECT"},              //80040206
	{DIERR_NOTBUFFERED,                 "DIERR_NOTBUFFERED"},                   //80040207
	{DIERR_EFFECTPLAYING,               "DIERR_EFFECTPLAYING"},                 //80040208
	{DIERR_UNPLUGGED,                   "DIERR_UNPLUGGED"},                     //80040209 ( DirectX 7.0 )
	{DIERR_REPORTFULL,                  "DIERR_REPORTFULL"}                     //8004020A ( DirectX 7.0 )
}; // end of "elDIErrors[]"

const int ERRDILISTLEN =  (sizeof(elDIErrors)/sizeof(ERRLIST));

/*———————————————————————————————————————————————————————————————————————————*
*  DIRECT PLAY ERRORS (direct play seems not to exist in recent directx sdk's) *
*———————————————————————————————————————————————————————————————————————————*/
/*
static ERRLIST elDPErrors[] = {
	{DPERR_ALREADYINITIALIZED,          "DPERR_ALREADYINITIALIZED"},            //   5
	{DPERR_ACCESSDENIED,                "DPERR_ACCESSDENIED"},                  //  10
	{DPERR_ACTIVEPLAYERS,               "DPERR_ACTIVEPLAYERS"},                 //  20
	{DPERR_BUFFERTOOSMALL,              "DPERR_BUFFERTOOSMALL"},                //  30
	{DPERR_CANTADDPLAYER,               "DPERR_CANTADDPLAYER"},                 //  40
	{DPERR_CANTCREATEGROUP,             "DPERR_CANTCREATEGROUP"},               //  50
	{DPERR_CANTCREATEPLAYER,            "DPERR_CANTCREATEPLAYER"},              //  60
	{DPERR_CANTCREATESESSION,           "DPERR_CANTCREATESESSION"},             //  70
	{DPERR_CAPSNOTAVAILABLEYET,         "DPERR_CAPSNOTAVAILABLEYET"},           //  80
	{DPERR_EXCEPTION,                   "DPERR_EXCEPTION"},                     //  90
	{DPERR_GENERIC,                     "DPERR_GENERIC"},                       //    E_FAIL
	{DPERR_INVALIDFLAGS,                "DPERR_INVALIDFLAGS"},                  // 120
	{DPERR_INVALIDOBJECT,               "DPERR_INVALIDOBJECT"},                 // 130
	{DPERR_INVALIDPARAM,                "DPERR_INVALIDPARAM"},                  //    E_INVALIDARG
	{DPERR_INVALIDPARAMS,               "DPERR_INVALIDPARAMS"},                 //    DPERR_INVALIDPARAM
	{DPERR_INVALIDPLAYER,               "DPERR_INVALIDPLAYER"},                 // 150
	{DPERR_INVALIDGROUP,                "DPERR_INVALIDGROUP"},                  // 155
	{DPERR_NOCAPS,                      "DPERR_NOCAPS"},                        // 160
	{DPERR_NOCONNECTION,                "DPERR_NOCONNECTION"},                  // 170
	{DPERR_NOMEMORY,                    "DPERR_NOMEMORY"},                      //    E_OUTOFMEMORY
	{DPERR_OUTOFMEMORY,                 "DPERR_OUTOFMEMORY"},                   //    DPERR_NOMEMORY
	{DPERR_NOMESSAGES,                  "DPERR_NOMESSAGES"},                    // 190
	{DPERR_NONAMESERVERFOUND,           "DPERR_NONAMESERVERFOUND"},             // 200
	{DPERR_NOPLAYERS,                   "DPERR_NOPLAYERS"},                     // 210
	{DPERR_NOSESSIONS,                  "DPERR_NOSESSIONS"},                    // 220
	{DPERR_PENDING,                     "DPERR_PENDING"},                       //    E_PENDING
	{DPERR_SENDTOOBIG,                  "DPERR_SENDTOOBIG"},                    // 230
	{DPERR_TIMEOUT,                     "DPERR_TIMEOUT"},                       // 240
	{DPERR_UNAVAILABLE,                 "DPERR_UNAVAILABLE"},                   // 250
	{DPERR_UNSUPPORTED,                 "DPERR_UNSUPPORTED"},                   //    E_NOTIMPL
	{DPERR_BUSY,                        "DPERR_BUSY"},                          // 270
	{DPERR_USERCANCEL,                  "DPERR_USERCANCEL"},                    // 280
	{DPERR_NOINTERFACE,                 "DPERR_NOINTERFACE"},                   //    E_NOINTERFACE
	{DPERR_CANNOTCREATESERVER,          "DPERR_CANNOTCREATESERVER"},            // 290
	{DPERR_PLAYERLOST,                  "DPERR_PLAYERLOST"},                    // 300
	{DPERR_SESSIONLOST,                 "DPERR_SESSIONLOST"},                   // 310
	{DPERR_UNINITIALIZED,               "DPERR_UNINITIALIZED"},                 // 320
	{DPERR_NONEWPLAYERS,                "DPERR_NONEWPLAYERS"},                  // 330
	{DPERR_INVALIDPASSWORD,             "DPERR_INVALIDPASSWORD"},               // 340
	{DPERR_CONNECTING,                  "DPERR_CONNECTING"},                    // 350
	{DPERR_CONNECTIONLOST,              "DPERR_CONNECTIONLOST"},                // 360 ( DirectX 7.0 )
	{DPERR_UNKNOWNMESSAGE,              "DPERR_UNKNOWNMESSAGE"},                // 370 ( DirectX 7.0 )
	{DPERR_CANCELFAILED,                "DPERR_CANCELFAILED"},                  // 380 ( DirectX 7.0 )
	{DPERR_INVALIDPRIORITY,             "DPERR_INVALIDPRIORITY"},               // 390 ( DirectX 7.0 )
	{DPERR_NOTHANDLED,                  "DPERR_NOTHANDLED"},                    // 400 ( DirectX 7.0 )
	{DPERR_CANCELLED,                   "DPERR_CANCELLED"},                     // 410 ( DirectX 7.0 )
	{DPERR_ABORTED,                     "DPERR_ABORTED"},                       // 420 ( DirectX 7.0 )
	{DPERR_BUFFERTOOLARGE,              "DPERR_BUFFERTOOLARGE"},                //1000
	{DPERR_CANTCREATEPROCESS,           "DPERR_CANTCREATEPROCESS"},             //1010
	{DPERR_APPNOTSTARTED,               "DPERR_APPNOTSTARTED"},                 //1020
	{DPERR_INVALIDINTERFACE,            "DPERR_INVALIDINTERFACE"},              //1030
	{DPERR_NOSERVICEPROVIDER,           "DPERR_NOSERVICEPROVIDER"},             //1040
	{DPERR_UNKNOWNAPPLICATION,          "DPERR_UNKNOWNAPPLICATION"},            //1050
	{DPERR_NOTLOBBIED,                  "DPERR_NOTLOBBIED"},                    //1070
	{DPERR_SERVICEPROVIDERLOADED,       "DPERR_SERVICEPROVIDERLOADED"},         //1080
	{DPERR_ALREADYREGISTERED,           "DPERR_ALREADYREGISTERED"},             //1090
	{DPERR_NOTREGISTERED,               "DPERR_NOTREGISTERED"},                 //1100
	{DPERR_AUTHENTICATIONFAILED,        "DPERR_AUTHENTICATIONFAILED"},          //2000
	{DPERR_CANTLOADSSPI,                "DPERR_CANTLOADSSPI"},                  //2010
	{DPERR_ENCRYPTIONFAILED,            "DPERR_ENCRYPTIONFAILED"},              //2020
	{DPERR_SIGNFAILED,                  "DPERR_SIGNFAILED"},                    //2030
	{DPERR_CANTLOADSECURITYPACKAGE,     "DPERR_CANTLOADSECURITYPACKAGE"},       //2040
	{DPERR_ENCRYPTIONNOTSUPPORTED,      "DPERR_ENCRYPTIONNOTSUPPORTED"},        //2050
	{DPERR_CANTLOADCAPI,                "DPERR_CANTLOADCAPI"},                  //2060
	{DPERR_NOTLOGGEDIN,                 "DPERR_NOTLOGGEDIN"},                   //2070
	{DPERR_LOGONDENIED,                 "DPERR_LOGONDENIED"}                    //2080
}; // end of "elDPErrors[]"
const int ERRDPLISTLEN =  (sizeof(elDPErrors)/sizeof(ERRLIST));

*/

static ERRLIST elXFErrors[] = {
	{DXFILEERR_BADOBJECT,"DXFILEERR_BADOBJECT"},                 // 850
	{DXFILEERR_BADVALUE,"DXFILEERR_BADVALUE"},                  // 851
	{DXFILEERR_BADTYPE,"DXFILEERR_BADTYPE"},                   // 852
	{DXFILEERR_BADSTREAMHANDLE,"DXFILEERR_BADSTREAMHANDLE"},           // 853
	{DXFILEERR_BADALLOC,"DXFILEERR_BADALLOC"},                  // 854
	{DXFILEERR_NOTFOUND,"DXFILEERR_NOTFOUND"},                  // 855
	{DXFILEERR_NOTDONEYET,"DXFILEERR_NOTDONEYET"},                // 856
	{DXFILEERR_FILENOTFOUND,"DXFILEERR_FILENOTFOUND"},              // 857
	{DXFILEERR_RESOURCENOTFOUND,"DXFILEERR_RESOURCENOTFOUND"},          // 858
	{DXFILEERR_URLNOTFOUND,"DXFILEERR_URLNOTFOUND"},               // 859
	{DXFILEERR_BADRESOURCE,"DXFILEERR_BADRESOURCE"},               // 860
	{DXFILEERR_BADFILETYPE,"DXFILEERR_BADFILETYPE"},               // 861
	{DXFILEERR_BADFILEVERSION,"DXFILEERR_BADFILEVERSION"},            // 862
	{DXFILEERR_BADFILEFLOATSIZE,"DXFILEERR_BADFILEFLOATSIZE"},          // 863
	{DXFILEERR_BADFILECOMPRESSIONTYPE,"DXFILEERR_BADFILECOMPRESSIONTYPE"},    // 864
	{DXFILEERR_BADFILE,"DXFILEERR_BADFILE"},                   // 865
	{DXFILEERR_PARSEERROR,"DXFILEERR_PARSEERROR"},                // 866
	{DXFILEERR_NOTEMPLATE,"DXFILEERR_NOTEMPLATE"},                // 867
	{DXFILEERR_BADARRAYSIZE,"DXFILEERR_BADARRAYSIZE"},              // 868
	{DXFILEERR_BADDATAREFERENCE,"DXFILEERR_BADDATAREFERENCE"},          // 869
	{DXFILEERR_INTERNALERROR,"DXFILEERR_INTERNALERROR"},             // 870
	{DXFILEERR_NOMOREOBJECTS,"DXFILEERR_NOMOREOBJECTS"},             // 871
	{DXFILEERR_BADINTRINSICS,"DXFILEERR_BADINTRINSICS"},             // 872
	{DXFILEERR_NOMORESTREAMHANDLES,"DXFILEERR_NOMORESTREAMHANDLES"},       // 873
	{DXFILEERR_NOMOREDATA,"DXFILEERR_NOMOREDATA"},                // 874
	{DXFILEERR_BADCACHEFILE,"DXFILEERR_BADCACHEFILE"},              // 875
	{DXFILEERR_NOINTERNET,"DXFILEERR_NOINTERNET"}                 // 876
};
const int ERRXFLISTLEN =  (sizeof(elXFErrors)/sizeof(ERRLIST));






/*—————————————————————————————————————————————————————————————————————————*
* Global Functions                                                        *
*—————————————————————————————————————————————————————————————————————————*/
/*—————————————————————————————————————————————————————————————————————————*
* Function: OpenDebugLogFile                                              *
* Purpose:                                                                *
*—————————————————————————————————————————————————————————————————————————*/
int OpenDebugLogFile(LPSTR lpszFileName)
{
	if(g_bLogFile)
	{
		iDebugLogFile = _open( lpszFileName, _O_WRONLY|_O_CREAT|_O_TRUNC,_S_IREAD|_S_IWRITE);
		return iDebugLogFile;
	}
	return -1;
}
/*—————————————————————————————————————————————————————————————————————————*
* Function: CloseDebugLogFile                                             *
* Purpose:                                                                *
*—————————————————————————————————————————————————————————————————————————*/
void CloseDebugLogFile( void )
{
	if(iDebugLogFile !=-1)
	{
		_close( iDebugLogFile );
	}
}
/*—————————————————————————————————————————————————————————————————————————*
* Function: WriteDebugLogFile                                             *
* Purpose:                                                                *
*—————————————————————————————————————————————————————————————————————————*/
int WriteDebugLogFile(LPSTR lpszComment)
{
	static int iGlobalCounter = 0;
	int iLenght = 0;
	char szLineMax[256];
	char szLine[256];

	if(iDebugLogFile == -1)
	{
		return -1;
	}
	if(iGlobalCounter > 999999)
	{
		return -1;
	}
	if(lpszComment[0] == '\0')
	{
		return -1;
	}

	iLenght = lstrlen(lpszComment);
	if(iLenght > 232 )
	{
		lstrcpyn(szLineMax,lpszComment,231);
		szLineMax[231] = '\0';
	}
	else
	{
		lstrcpy(szLineMax,lpszComment);
	}
	sprintf(szLine,"%6d: %s\r\n ",iGlobalCounter,szLineMax);
	iLenght = _write( iDebugLogFile, szLine, lstrlen(szLine) );
	_commit( iDebugLogFile );
	iGlobalCounter++;
	return iLenght;
}

/*—————————————————————————————————————————————————————————————————————————*
* Function: TRACE                                                         *
* Purpose:                                                                *
*—————————————————————————————————————————————————————————————————————————*/
void DUMP_F(char * fmt, ...)
{
	const int nBufferSize = 256;
	char ach[nBufferSize + 8];
	va_list va;

	va_start( va, fmt );
	_vsnprintf(ach, nBufferSize, fmt, va);
	va_end( va );
	OutputDebugString(ach);
	OutputDebugString("\n");
	if( iDebugLogFile != -1 )
	{
		WriteDebugLogFile( ach );
	}
}
/*—————————————————————————————————————————————————————————————————————————*
* Function: THERC                                                         *
* Purpose:                                                                *
*—————————————————————————————————————————————————————————————————————————*/
#define __JJJG__    // MJ
void DUMP_O(char * fmt, ...)
{
#ifdef __JJJG__    // MJ
	const int nBufferSize = 256;
	char ach[nBufferSize + 8];
	va_list va;
	va_start( va, fmt );
	_vsnprintf(ach, nBufferSize, fmt, va);
	va_end( va );
	OutputDebugString("D_LOG: ");
	OutputDebugString(ach);
	OutputDebugString("\n");
#endif
}

/*—————————————————————————————————————————————————————————————————————————*
* Function: TR_DX                                                         *
* Purpose:                                                                *
*—————————————————————————————————————————————————————————————————————————*/
void TR_DX(HRESULT hResult)
{

	if( hResult != DD_OK)
	{
		char ach[256];
		sprintf( ach,"DX Error: %s",GetDirectXStatusString1(hResult));
		OutputDebugString("D_LOG: ");
		OutputDebugString(ach);
		OutputDebugString(" \r\n");
		if( iDebugLogFile != -1 )
		{
			WriteDebugLogFile( ach );
		}
	}
}

/*—————————————————————————————————————————————————————————————————————————*
* Function: GetDirectXStatusString                                        *
* Purpose:                                                                *
*—————————————————————————————————————————————————————————————————————————*/

LPSTR GetDirectXStatusString1(HRESULT DDStatus)
{
	int i = 0;

	// 1. Direct Draw Error
	for(i = 0; i < ERRLISTLEN; i++)
	{
		if(DDStatus == elDDErrors[i].hResult)
		{
			return(elDDErrors[i].lpszError);
		}
	}

	// 2. Direct 3D Error
	for(i = 0; i < ERRD3DLISTLEN; i++)
	{
		if(DDStatus == elD3DErrors[i].hResult)
		{
			return(elD3DErrors[i].lpszError);
		}
	}

	// 3. Direct Sound Error
	for(i = 0; i < ERRDSLISTLEN; i++)
	{
		if(DDStatus == elDSErrors[i].hResult)
		{
			return(elDSErrors[i].lpszError);
		}
	}

	// 4. Direct Input Error
	for(i = 0; i < ERRDILISTLEN; i++)
	{
		if(DDStatus == elDIErrors[i].hResult)
		{
			return(elDIErrors[i].lpszError);
		}
	}

	// 5. Direct Play Error
	//for(i = 0; i < ERRDPLISTLEN; i++)
	//{
	//	if(DDStatus == elDPErrors[i].hResult)
	//	{
	//		return(elDPErrors[i].lpszError);
	//	}
	//}

	// 6. Direct X File Error
	for(i = 0; i < ERRXFLISTLEN; i++)
	{
		if(DDStatus == elXFErrors[i].hResult)
		{
			return(elXFErrors[i].lpszError);
		}
	}
	//FAKE: We always use plain ascii character sets here.
	return "Unknown error code";

	//return(dxerr_unknown);
}
