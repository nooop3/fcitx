/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*
 * @file   ime.c
 * @author Yuking yuking_net@sohu.com
 * @date   2008-1-16
 *
 * @brief  按键和输入法通用功能处理
 *
 *
 */

#include <ctype.h>
#include <time.h>

#include "core/xim.h"
#include "core/ime.h"
#include "core/IC.h"
#include "ui/about.h"
#include "ui/InputWindow.h"
#include "ui/MainWindow.h"
#include "ui/TrayWindow.h"
#include "ui/font.h"
#include "ui/ui.h"
#include "im/special/punc.h"
#include "im/pinyin/py.h"
#include "im/pinyin/sp.h"
#include "im/qw/qw.h"
#include "im/table/table.h"
#include "im/special/vk.h"
#include "im/special/QuickPhrase.h"
#include "im/special/AutoEng.h"
#include "im/extra/extra.h"
#include "ui/skin.h"
#include "tools/tools.h"
#include "interface/DBus.h"
#include "fcitx-config/profile.h"

FcitxState gs; /* global state */

IM             *im = NULL;
INT8            iIMCount = 0;
INT8		iState = 0;

int             iMaxCandWord = 5;
int             iCandPageCount;
int             iCurrentCandPage;
int             iCandWordCount;

int             iLegendCandWordCount;
int             iLegendCandPageCount;
int             iCurrentLegendCandPage;

int             iCodeInputCount;

// *************************************************************
char            strCodeInput[MAX_USER_INPUT + 1];
char            strStringGet[MAX_USER_INPUT + 1];	//保存输入法返回的需要送到客户程序中的字串

// *************************************************************

ENTER_TO_DO     enterToDo = K_ENTER_SEND;

Bool            bIsDoInputOnly = False;	//表明是否只由输入法来处理键盘
Bool            bLastIsNumber = False;	//上一次输入是不是阿拉伯数字
char		cLastIsAutoConvert = 0;	//上一次输入是不是符合数字后自动转换的符号，如'.'/','，0表示不是这样的符号
INT8            iInCap = 0;		//是不是处于大写后的英文状态,0--不，1--按下大写键，2--按下分号键

/*
Bool            bAutoHideInputWindow = False;	//是否自动隐藏输入条
*/
Bool            bEngPuncAfterNumber = True;	//数字后面输出半角符号(只对'.'/','有效)
Bool            bPhraseTips = True;
INT8            lastIsSingleHZ = 0;

SEMICOLON_TO_DO semicolonToDo = K_SEMICOLON_QUICKPHRASE;
Bool            bEngAfterCap = True;
Bool            bConvertPunc = True;
Bool            bDisablePagingInLegend = True;
Bool            bSendTextWhenSwitchEng = False;

Bool            bVK = False;

int             i2ndSelectKey = 50;	//第二个候选词选择键，为扫描码-默认是SHIFT
int             i3rdSelectKey = 62;	//第三个候选词选择键，为扫描码

Time            lastKeyPressedTime;
unsigned int    iTimeInterval = 250;

KEY_RELEASED    keyReleased = KR_OTHER;
Bool            bDoubleSwitchKey = False;
KEY_CODE        switchKey = L_CTRL;

//热键定义
HOTKEYS         hkTrigger[HOT_KEY_COUNT] = { CTRL_SPACE, 0 };
HOTKEYS         hkLegend[HOT_KEY_COUNT] = { CTRL_L, 0 };
HOTKEYS         hkCorner[HOT_KEY_COUNT] = { SHIFT_SPACE, 0 };	//全半角切换
HOTKEYS         hkPunc[HOT_KEY_COUNT] = { ALT_SPACE, 0 };	//中文标点
HOTKEYS         hkNextPage[HOT_KEY_COUNT] = { '.', 0 };	//下一页
HOTKEYS         hkPrevPage[HOT_KEY_COUNT] = { ',', 0 };	//上一页
HOTKEYS         hkTrack[HOT_KEY_COUNT] = { CTRL_K, 0 };
HOTKEYS         hkGBT[HOT_KEY_COUNT] = { CTRL_ALT_F, 0 };
HOTKEYS         hkHideMainWindow[HOT_KEY_COUNT] = { CTRL_ALT_H, 0 };
HOTKEYS         hkSaveAll[HOT_KEY_COUNT] = { CTRL_ALT_S, 0 };
HOTKEYS         hkVK[HOT_KEY_COUNT] = { CTRL_ALT_K, 0 };

#ifdef _ENABLE_RECORDING
HOTKEYS         hkRecording[HOT_KEY_COUNT] = { CTRL_ALT_J, 0 };
HOTKEYS		hkResetRecording[HOT_KEY_COUNT] = { CTRL_ALT_A, 0 };
#endif

Bool            bIsInLegend = False;

// dgod extern im
char		strExternIM[PATH_MAX];

Bool            bPointAfterNumber = True;

/* 计算打字速度 */
time_t          timeStart;
Bool            bStartRecordType;
Bool            bShowUserSpeed = True;
Bool            bShowVersion = True;
uint            iHZInputed = 0;

Bool		bCursorAuto=False;

int		inputMethods[INPUT_METHODS];

extern XIMS     ims;
extern Display *dpy;
extern ChnPunc *chnPunc;

extern Bool     bShowPrev;
extern Bool     bShowNext;
extern Bool     bShowCursor;

extern Window   inputWindow;
extern HIDE_MAINWINDOW hideMainWindow;
extern XIMTriggerKey *Trigger_Keys;
extern Window   mainWindow;
extern int      iCursorPos;

extern VKWindow   vkWindow;
extern VKS      vks[];
extern unsigned char iCurrentVK;
extern Bool     bVK;

extern Window   aboutWindow;

extern Bool     bShowVK;

extern INT8     iTableChanged;
extern INT8     iNewPYPhraseCount;
extern INT8     iOrderCount;
extern INT8     iNewFreqCount;

extern INT8     iTableCount;

extern Bool     bTrigger;

extern int      iInputWindowX;
extern int      iInputWindowY;

extern Bool     bShowInputWindowTriggering;
extern Bool	bMainWindow_Hiden;
extern char    *strFullCorner;

extern Bool	bUseDBus;
extern CARD16	connect_id;

extern int iFirstQuickPhrase;

#ifdef _ENABLE_DBUS
extern Property logo_prop;
extern Property state_prop;
extern Property punc_prop;
extern Property corner_prop;
extern Property gbkt_prop;
extern Property legend_prop;
#endif


#ifdef _ENABLE_RECORDING
extern FILE	*fpRecord;
extern Bool     bWrittenRecord;
#endif

char *sCornerTrans[] = {
    "　", "！", "＂", "＃", "￥", "％", "＆", "＇", "（", "）", "＊", "＋", "，", "－", "．", "／", 
    "０", "１", "２", "３", "４", "５", "６", "７", "８", "９", "：", "；", "＜", "＝", "＞", "？",
    "＠", "Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ", "Ｆ", "Ｇ", "Ｈ", "Ｉ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ", "Ｏ",
    "Ｐ", "Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ", "Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ", "［", "＼", "］", "＾", "＿",
    "｀", "ａ", "ｂ", "ｃ", "ｄ", "ｅ", "ｆ", "ｇ", "ｈ", "ｉ", "ｊ", "ｋ", "ｌ", "ｍ", "ｎ", "ｏ",
    "ｐ", "ｑ", "ｒ", "ｓ", "ｔ", "ｕ", "ｖ", "ｗ", "ｘ", "ｙ", "ｚ", "｛", "｜", "｝", "￣",
};

void ResetInput (void)
{
    iCandPageCount = 0;
    iCurrentCandPage = 0;
    iCandWordCount = 0;
    iLegendCandWordCount = 0;
    iCurrentLegendCandPage = 0;
    iLegendCandPageCount = 0;
    iCursorPos = 0;

    strCodeInput[0] = '\0';
    iCodeInputCount = 0;

    bIsDoInputOnly = False;

    bShowPrev = False;
    bShowNext = False;

    bIsInLegend = False;
    iInCap = 0;

    if (!IsIM (strNameOfPinyin))
    bShowCursor = False;

    if (im[gs.iIMIndex].ResetIM)
    im[gs.iIMIndex].ResetIM ();

    iFirstQuickPhrase = -1;
}

void CloseIM (IMForwardEventStruct * call_data)
{
    CloseInputWindow();
    
    XUnmapWindow (dpy, vkWindow.window);

    IMPreeditEnd (ims, (XPointer) call_data);
    SetConnectID (call_data->connect_id, IS_CLOSED);
    icidSetIMState(call_data->icid, IS_CLOSED);
    bVK = False;
    SwitchIM (-2);
    
    if (!bUseDBus) {
    DrawMainWindow ();

#ifdef _ENABLE_TRAY
    DrawTrayWindow (INACTIVE_ICON, 0, 0, tray.size, tray.size );
#endif
    }
#ifdef _ENABLE_DBUS
    else
    updateProperty(&state_prop);
#endif
}

void ChangeIMState (CARD16 _connect_id)
{
    if (ConnectIDGetState (_connect_id) == IS_ENG) {
    SetConnectID (_connect_id, IS_CHN);
    iState = IS_CHN;

    if (bVK) {
        DisplayVKWindow ();
    }
    else
        DisplayInputWindow ();
    }
    else {
    SetConnectID (_connect_id, IS_ENG);
    iState = IS_ENG;
    ResetInput ();
    ResetInputWindow ();

    CloseInputWindow();
    }
    XUnmapWindow (dpy, vkWindow.window);
    
    if (!bUseDBus) {
    if (hideMainWindow != HM_HIDE)
        DrawMainWindow ();
    }

#ifdef _ENABLE_DBUS 
    if (bUseDBus) {
    updateProperty(&state_prop);
    }
#endif
}

/*
 * 转换strStringGet中的标点为中文标点
 */
void ConvertPunc (void)
{
    char            strTemp[MAX_USER_INPUT + 1] = "\0";
    char           *s1, *s2, *pPunc;

    s1 = strTemp;
    s2 = strStringGet;

    while (*s2) {
    pPunc = GetPunc (*s2);
    if (pPunc) {
        strcat (s1, pPunc);
        s1 += strlen (pPunc);
    }
    else
        *s1++ = *s2;
    s2++;
    }
    *s2 = '\0';

    strcpy (strStringGet, strTemp);
}

void ProcessKey (IMForwardEventStruct * call_data)
{
    KeySym          keysym;
    XKeyEvent      *kev;
    int             keyCount;
    INPUT_RETURN_VALUE retVal;
    int             iKeyState;
    char            strbuf[STRBUFLEN];
    int             iKey;
    char           *pstr;
    int             iLen;

    kev = (XKeyEvent *) & call_data->event;
    memset (strbuf, 0, STRBUFLEN);
    keyCount = XLookupString (kev, strbuf, STRBUFLEN, &keysym, NULL);

    iKeyState = kev->state - (kev->state & KEY_NUMLOCK) - (kev->state & KEY_CAPSLOCK) - (kev->state & KEY_SCROLLLOCK);
    iKey = GetKey (keysym, iKeyState, keyCount);

    if (!iKey) {
    IMForwardEvent (ims, (XPointer) call_data);
    return;
    }

    /*
     * 原意是为了解决xine-ui中候选字自动选中的问题
     * xine-ui每秒钟产生一个左SHIFT键的释放事件
     * 但这段代码对新的xine-ui已经不起作用了
     */
     /*if (kev->same_screen && (kev->keycode == switchKey || kev->keycode == i2ndSelectKey || kev->keycode == i3rdSelectKey)) {
    IMForwardEvent (ims, (XPointer) call_data);
    return;
     }*/

    retVal = IRV_TO_PROCESS;

#ifdef _DEBUG
    printf ("KeyRelease=%d  iKeyState=%d  KEYCODE=%d  KEYSYM=%d  keyCount=%d  iKey=%d\n", (call_data->event.type == KeyRelease), iKeyState, (int) keysym, kev->keycode, keyCount, iKey);
#endif

    /* Added by hubert_star AT forum.ubuntu.com.cn */
    if ( (call_data->event.type == KeyRelease) && (((iKey >= 32 ) && (iKey <= 126)) && (iKey != (i2ndSelectKey ^ 0xFF)) && (iKey != (i3rdSelectKey ^ 0xFF))))
        return;

    /* ******************************************* */
    if (call_data->event.type == KeyRelease) {
    if (ConnectIDGetState (call_data->connect_id) != IS_CLOSED) {
        if ((kev->time - lastKeyPressedTime) < 500 && (!bIsDoInputOnly)) {
        if (iKeyState == KEY_CTRL_SHIFT_COMP && (iKey == 225 || iKey == 227)) {
            if ( !fcitxProfile.bLocked ) {
                if (ConnectIDGetState (call_data->connect_id) == IS_CHN)
        	    SwitchIM (-1);
                else if (IsHotKey (iKey, hkTrigger))
        	    CloseIM (call_data);
            }
            else if ( bVK )
                ChangVK();
        }
        else if (iKey == CTRL_LSHIFT) {
            if ( !fcitxProfile.bLocked ) {
                if (ConnectIDGetState (call_data->connect_id) == IS_CHN)
        	    SwitchIM (-1);
                else if (IsHotKey (iKey, hkTrigger))
        	    CloseIM (call_data);
            }
            else if ( bVK )
                ChangVK();
        }
        else if (kev->keycode == switchKey && keyReleased == KR_CTRL && !bDoubleSwitchKey) {
            retVal = IRV_DONOT_PROCESS;
            if (bSendTextWhenSwitchEng)
            {
                if (iCodeInputCount) {
                    strcpy (strStringGet, strCodeInput);
                    retVal = IRV_ENG;
                }
            }
            keyReleased = KR_OTHER;
            ChangeIMState (call_data->connect_id);
            }
        else if ((kev->keycode == i2ndSelectKey && keyReleased == KR_2ND_SELECTKEY)
        	 || (iKey == (i2ndSelectKey ^ 0xFF) && keyReleased == KR_2ND_SELECTKEY_OTHER)) {
            if (!bIsInLegend) {
        	pstr = im[gs.iIMIndex].GetCandWord (1);
        	if (pstr) {
        	    strcpy (strStringGet, pstr);
        	    if (bIsInLegend)
        		retVal = IRV_GET_LEGEND;
        	    else
        		retVal = IRV_GET_CANDWORDS;
        	}
        	else if (iCandWordCount)
        	    retVal = IRV_DISPLAY_CANDWORDS;
        	else
        	    retVal = IRV_TO_PROCESS;
            }
            else {
        	strcpy (strStringGet, " ");
        	SetMessageCount(&messageDown, 0);
        	retVal = IRV_GET_CANDWORDS;
            }

            keyReleased = KR_OTHER;
        }
        else if ((kev->keycode == i3rdSelectKey && keyReleased == KR_3RD_SELECTKEY)
        	 || (iKey == (i3rdSelectKey ^ 0xFF) && keyReleased == KR_3RD_SELECTKEY_OTHER) ) {
            if (!bIsInLegend) {
        	pstr = im[gs.iIMIndex].GetCandWord (2);
        	if (pstr) {
        	    strcpy (strStringGet, pstr);
        	    if (bIsInLegend)
        		retVal = IRV_GET_LEGEND;
        	    else
        		retVal = IRV_GET_CANDWORDS;
        	}
        	else if (iCandWordCount)
        	    retVal = IRV_DISPLAY_CANDWORDS;
            }
            else {
        	strcpy (strStringGet, "　");
        	SetMessageCount(&messageDown, 0);
        	retVal = IRV_GET_CANDWORDS;
            }

            keyReleased = KR_OTHER;
        }
        }
    }
    else
        call_data->event.type = KeyPress;
    }

    if (retVal == IRV_TO_PROCESS) {
    if (call_data->event.type == KeyPress) {
        if (kev->keycode != switchKey)
        keyReleased = KR_OTHER;
        else {
        if ((keyReleased == KR_CTRL) && (kev->time - lastKeyPressedTime < iTimeInterval) && bDoubleSwitchKey) {
                SendHZtoClient(call_data, strCodeInput);
            ChangeIMState (call_data->connect_id);
            }
        }

        lastKeyPressedTime = kev->time;
        if (kev->keycode == switchKey) {
        keyReleased = KR_CTRL;
        retVal = IRV_DO_NOTHING;
        }
        else if (IsHotKey (iKey, hkTrigger)) {
        if (ConnectIDGetState (call_data->connect_id) == IS_ENG) {
            SetConnectID (call_data->connect_id, IS_CHN);

            EnterChineseMode (False);
            if (!bUseDBus)
        	DrawMainWindow ();

            if (bShowInputWindowTriggering && !fcitxProfile.bCorner) {
        	    DisplayInputWindow ();
            }
            else
                MoveInputWindow(call_data->connect_id);
        }
        else
            CloseIM (call_data);

        retVal = IRV_DO_NOTHING;
        }
    }

    if (retVal == IRV_TO_PROCESS) {
        if (call_data->event.type == KeyPress) {
        if (ConnectIDGetState (call_data->connect_id) == IS_CHN) {
            if (bVK)
        	retVal = DoVKInput (iKey);
            else {
        	if (iKeyState == KEY_NONE) {
        	    if (kev->keycode == i2ndSelectKey) {
        		keyReleased = KR_2ND_SELECTKEY;
        		return;
        	    }
        	    else if (kev->keycode == i3rdSelectKey) {
        		keyReleased = KR_3RD_SELECTKEY;
        		return;
        	    }
        	    else if (iKey == (i2ndSelectKey ^ 0xFF)) {
        		if (iCandWordCount >= 2) {
        		    keyReleased = KR_2ND_SELECTKEY_OTHER;
        		    return;
        		}
        	    }
        	    else if (iKey == (i3rdSelectKey ^ 0xFF)) {
        		if (iCandWordCount >= 2) {
        		    keyReleased = KR_3RD_SELECTKEY_OTHER;
        		    return;
        		}
        	    }
        	}

        	if (iKey == CTRL_LSHIFT || iKey == SHIFT_LCTRL) {
        	    if (fcitxProfile.bLocked)
        		retVal = IRV_TO_PROCESS;
        	}
        	else {
        	    //调用输入法模块
        	    if (fcitxProfile.bCorner && (iKey >= 32 && iKey <= 126)) {
        		//有人报 空格 的全角不对，正确的是0xa1 0xa1
        		//但查资料却说全角符号总是以0xa3开始。
        		//由于0xa3 0xa0可能会显示乱码，因此采用0xa1 0xa1的方式
        		sprintf (strStringGet, "%s", sCornerTrans[iKey - 32]);
        		retVal = IRV_GET_CANDWORDS;
        	    }
        	    else {
        		if (!iInCap) {
        		    char            strTemp[MAX_USER_INPUT];

        		    retVal = im[gs.iIMIndex].DoInput (iKey);
        		    if (!bCursorAuto && !IsIM (strNameOfPinyin) && !IsIM (strNameOfShuangpin))
        			iCursorPos = iCodeInputCount;

        		    //为了实现自动英文转换
        		    strcpy (strTemp, strCodeInput);
        		    if (retVal == IRV_TO_PROCESS) {
        			strTemp[strlen (strTemp) + 1] = '\0';
        			strTemp[strlen (strTemp)] = iKey;
        		    }

        		    if (SwitchToEng (strTemp)) {
        			iInCap = 3;
        			if (retVal != IRV_TO_PROCESS) {
        			    iCodeInputCount--;
        			    retVal = IRV_TO_PROCESS;
        			}
        		    }

        		    if (iKey!= (XK_BackSpace & 0x00FF))
        			cLastIsAutoConvert = 0;
        		}
        		else if (iInCap == 2 && semicolonToDo == K_SEMICOLON_QUICKPHRASE && !iLegendCandWordCount)
        		    retVal = QuickPhraseDoInput (iKey);

        		if (!bIsDoInputOnly && retVal == IRV_TO_PROCESS) {
        		    if (!iInCap && iKey >= 'A' && iKey <= 'Z' && bEngAfterCap && !(kev->state & KEY_CAPSLOCK)) {
        			iInCap = 1;
        			if (!bIsInLegend && iCandWordCount) {
        			    pstr = im[gs.iIMIndex].GetCandWord (0);
        			    iCandWordCount = 0;
        			    if (pstr) {
        				SendHZtoClient (call_data, pstr);
        				strcpy (strStringGet, pstr);
        				//粗略统计字数
        				iHZInputed += (int) (utf8_strlen (strStringGet));
        				iCodeInputCount = 0;
        			    }
        			}
        		    }
        		    else if (iKey == ';' && semicolonToDo != K_SEMICOLON_NOCHANGE && !iCodeInputCount) {
        			if (iInCap != 2)
        			    iInCap = 2;
        			else
        			    iKey = ' ';	//使用第2个分号输入中文分号
        		    }
        		    else if (!iInCap) {
        			if (IsHotKey (iKey, hkPrevPage))
        			    retVal = im[gs.iIMIndex].GetCandWords (SM_PREV);
        			else if (IsHotKey (iKey, hkNextPage))
        			    retVal = im[gs.iIMIndex].GetCandWords (SM_NEXT);
        		    }

        		    if (retVal == IRV_TO_PROCESS) {
        			if (iInCap) {
        			    if ((iKey == ' ') && (iCodeInputCount == 0)) {
        				strcpy (strStringGet, "；");
        				retVal = IRV_ENG;
                        SetMessageCount(&messageDown, 0);
                        SetMessageCount(&messageUp, 0);
        				iInCap = 0;
        			    }
        			    else {
        				if (isprint (iKey) && iKey < 128) {
        				    if (iCodeInputCount == MAX_USER_INPUT)
        					retVal = IRV_DO_NOTHING;
        				    else {
        					if (!(iInCap == 2 && !iCodeInputCount && iKey == ';')) {
        					    strCodeInput[iCodeInputCount++] = iKey;
        					    strCodeInput[iCodeInputCount] = '\0';
        					    bShowCursor = True;
        					    iCursorPos = iCodeInputCount;
        					    if (semicolonToDo == K_SEMICOLON_QUICKPHRASE && iInCap == 2)
                                {
                                    if (iFirstQuickPhrase == -1)
                                        retVal = QuickPhraseGetCandWords (SM_FIRST);
                                    else
                                    {
                                        if (IsHotKey (iKey, hkPrevPage) || IsHotKey (iKey, hkNextPage))
                                        {
                                            if (iCodeInputCount)
                                                iCodeInputCount--;
                                            strCodeInput[iCodeInputCount] = '\0';
                                            iCursorPos = iCodeInputCount;
                                        }

                                        if (IsHotKey (iKey, hkPrevPage))
                                            retVal = QuickPhraseGetCandWords (SM_PREV);
                                        else if (IsHotKey (iKey, hkNextPage))
                                            retVal = QuickPhraseGetCandWords (SM_NEXT);
                                        else
                                            retVal = QuickPhraseGetCandWords (SM_FIRST);
                                    }
                                }
                                else
                                    retVal = IRV_DISPLAY_MESSAGE;
        					}
        					else
        					    retVal = IRV_DISPLAY_MESSAGE;
        				    }
        				}
        				else if (iKey == (XK_BackSpace & 0x00FF) || iKey == CTRL_H) {
        				    if (iCodeInputCount)
        					iCodeInputCount--;
        				    strCodeInput[iCodeInputCount] = '\0';
        				    iCursorPos = iCodeInputCount;
        				    if (!iCodeInputCount)
        					retVal = IRV_CLEAN;
        				    else if (semicolonToDo == K_SEMICOLON_QUICKPHRASE && iInCap == 2)
                            {
                                if (iFirstQuickPhrase == -1)
                                    retVal = QuickPhraseGetCandWords (SM_FIRST);
                                else if (IsHotKey (iKey, hkPrevPage))
                                    retVal = QuickPhraseGetCandWords (SM_PREV);
                                else if (IsHotKey (iKey, hkNextPage))
                                    retVal = QuickPhraseGetCandWords (SM_NEXT);
                                else
                                    retVal = QuickPhraseGetCandWords (SM_FIRST);
                           }
        				    else
        					retVal = IRV_DISPLAY_MESSAGE;
        				}

        				SetMessageCount(&messageUp, 0);
        				if (iInCap == 2) {
        				    if (semicolonToDo == K_SEMICOLON_ENG) {
                                AddMessageAtLast(&messageUp, MSG_TIPS, "英文输入 ");
                                iCursorPos += strlen("英文输入 ");
        				    }
        				    else {
                                AddMessageAtLast(&messageUp, MSG_TIPS, "自定义输入 ");
                                iCursorPos += strlen("自定义输入 ");
        				    }

        				    if (iCodeInputCount) {
                                AddMessageAtLast(&messageUp, MSG_INPUT, strCodeInput);
        				    }

        				    if (retVal != IRV_DISPLAY_CANDWORDS) {
        					SetMessageCount(&messageDown, 0);
        					if (iCodeInputCount)
                                AddMessageAtLast(&messageDown, MSG_TIPS, "按 Enter 输入英文");
        					else
                                AddMessageAtLast(&messageDown, MSG_TIPS, "空格输入；Enter输入;");
        				    }
        				}
        				else {
                            SetMessageCount(&messageDown, 0);
                            AddMessageAtLast(&messageDown, MSG_TIPS, "按 Enter 输入英文");
                            AddMessageAtLast(&messageUp, MSG_INPUT, strCodeInput);
        				}
        			    }
        			}
        			else if ((bLastIsNumber && bEngPuncAfterNumber) && (iKey == '.' || iKey == ',' || iKey == ':') && !iCandWordCount) {
        			    cLastIsAutoConvert = iKey;
        			    bLastIsNumber = False;
        			    retVal = IRV_TO_PROCESS;
        			}
        			else {
        			    if (fcitxProfile.bChnPunc) {
        				char           *pPunc;

        				pstr = NULL;
        				pPunc = GetPunc (iKey);
        				if (pPunc) {
        				    strStringGet[0] = '\0';
        				    if (!bIsInLegend)
        					pstr = im[gs.iIMIndex].GetCandWord (0);
        				    if (pstr)
        					strcpy (strStringGet, pstr);
        				    strcat (strStringGet, pPunc);
        				    SetMessageCount(&messageDown, 0);
        				    SetMessageCount(&messageUp, 0);

        				    retVal = IRV_PUNC;
        				}
        				else if ((iKey == (XK_BackSpace & 0x00FF) || iKey == CTRL_H) && cLastIsAutoConvert ) {
        				    char *pPunc;

        				    IMForwardEvent (ims, (XPointer) call_data);
        				    pPunc = GetPunc(cLastIsAutoConvert);
        				    if ( pPunc )
        				        SendHZtoClient(call_data, pPunc);

        				    retVal = IRV_DO_NOTHING;
        			        }
        				else if (isprint (iKey) && iKey < 128) {
        				    if (iKey >= '0' && iKey <= '9')
        					bLastIsNumber = True;
        				    else {
        					bLastIsNumber = False;
        					if (iKey == ' ')
        					    retVal = IRV_DONOT_PROCESS_CLEAN;	//为了与mozilla兼容
        					else {
        					    strStringGet[0] = '\0';
        					    if (!bIsInLegend)
        						pstr = im[gs.iIMIndex].GetCandWord (0);
        					    if (pstr)
        						strcpy (strStringGet, pstr);
        					    iLen = strlen (strStringGet);
                                SetMessageCount(&messageDown, 0);
                                SetMessageCount(&messageUp, 0);
        					    strStringGet[iLen] = iKey;
        					    strStringGet[iLen + 1] = '\0';
        					    retVal = IRV_ENG;
        					}
        				    }
        				}
        			    }
        			    cLastIsAutoConvert = 0;
        			}
        		    }
        		}

        		if (retVal == IRV_TO_PROCESS) {
        		    if (iKey == ESC) {
        			if (iCodeInputCount || iInCap || bIsInLegend)
        			    retVal = IRV_CLEAN;
        			else
        			    retVal = IRV_DONOT_PROCESS;
        		    }
        		    else if (iKey == CTRL_5) {
        			LoadConfig (False);

        			if (!bUseDBus) {
        			    if (!mainWindow)
        			        CreateMainWindow();
        			    if (hideMainWindow != HM_HIDE) {
        				DisplayMainWindow ();
        				DrawMainWindow ();
        			    }
#ifdef _ENABLE_TRAY
        			    if (!tray.window) {
        			        CreateTrayWindow();
        			        DrawTrayWindow (INACTIVE_ICON,0,0,tray.size, tray.size);
        			    }
#endif
        			    if (!aboutWindow)
        				CreateAboutWindow();
        			    InitMainWindowColor ();
        			    InitInputWindowColor ();
        			}
        			else {
        			    XUnmapWindow(dpy, mainWindow);
#ifdef _ENABLE_TRAY
        			    XDestroyWindow(dpy,tray.window);
        			    tray.window = (Window) NULL;
        			    tray.bTrayMapped = False;
#endif
        			}

        			SetIM ();
        			if (!bUseDBus) {
                        CreateFont();
        			    CalculateInputWindowHeight ();
        			}

        			FreeQuickPhrase ();
        			LoadQuickPhrase ();

        			FreeAutoEng ();
        			LoadAutoEng ();

        			FreePunc ();
        			LoadPuncDict ();
        			SwitchIM(-2);
        			if (!bUseDBus)
        			    DrawMainWindow();

        			retVal = IRV_DO_NOTHING;
        		    }
        		    else if (iKey == ENTER_K) {
        			if (iInCap) {
        			    if (!iCodeInputCount)
        				strcpy (strStringGet, ";");
        			    else
        				strcpy (strStringGet, strCodeInput);
        			    retVal = IRV_PUNC;
                        SetMessageCount(&messageDown, 0);
                        SetMessageCount(&messageUp, 0);
        			    iInCap = 0;
        			}
        			else if (!iCodeInputCount)
        			    retVal = IRV_DONOT_PROCESS;
        			else {
        			    switch (enterToDo) {
        			    case K_ENTER_NOTHING:
        				retVal = IRV_DO_NOTHING;
        				break;
        			    case K_ENTER_CLEAN:
        				retVal = IRV_CLEAN;
        				break;
        			    case K_ENTER_SEND:
                        SetMessageCount(&messageDown, 0);
                        SetMessageCount(&messageUp, 0);
        				strcpy (strStringGet, strCodeInput);
        				retVal = IRV_ENG;
        				break;
        			    }
        			}
        		    }
        		    else if (isprint (iKey) && iKey < 128)
        			retVal = IRV_DONOT_PROCESS_CLEAN;
        		    else
        			retVal = IRV_DONOT_PROCESS;
        		}
        	    }
        	}
            }
        }

        if (retVal == IRV_TO_PROCESS || retVal == IRV_DONOT_PROCESS) {
            if (IsHotKey (iKey, hkCorner))
        	retVal = ChangeCorner ();
            else if (IsHotKey (iKey, hkPunc))
        	retVal = ChangePunc ();
            else if (IsHotKey (iKey, hkLegend))
        	retVal = ChangeLegend ();
            else if (IsHotKey (iKey, hkTrack))
        	retVal = ChangeTrack ();
            else if (IsHotKey (iKey, hkGBT))
        	retVal = ChangeGBKT ();
            else if (IsHotKey (iKey, hkHideMainWindow)) {
        	if (bMainWindow_Hiden) {
        	    bMainWindow_Hiden = False;
        	    if (!bUseDBus) {
        		DisplayMainWindow();
        		DrawMainWindow();
        	    }
                }
        	else {
        	    bMainWindow_Hiden = True;
        	    if (!bUseDBus) 
        		XUnmapWindow(dpy,mainWindow);
        	}
        	retVal = IRV_DO_NOTHING;
            }
            else if (IsHotKey (iKey, hkSaveAll)) {
        	SaveIM();
            SetMessageCount(&messageDown, 0);
            AddMessageAtLast(&messageDown, MSG_TIPS, "词库已保存");
        	retVal = IRV_DISPLAY_MESSAGE;
            }
            else if (IsHotKey (iKey, hkVK) ) 
                SwitchVK ();
#ifdef _ENABLE_RECORDING
            else if (IsHotKey (iKey, hkRecording) )
                ChangeRecording ();
            else if (IsHotKey (iKey, hkResetRecording) )
                ResetRecording ();
#endif
        }
        }
        else
        retVal = IRV_DONOT_PROCESS;
    }
    }
    
    switch (retVal) {
    case IRV_DO_NOTHING:
    break;
    case IRV_TO_PROCESS:
    case IRV_DONOT_PROCESS:
    case IRV_DONOT_PROCESS_CLEAN:
    IMForwardEvent (ims, (XPointer) call_data);

    if (retVal != IRV_DONOT_PROCESS_CLEAN)
        return;
    case IRV_CLEAN:
    ResetInput ();
    ResetInputWindow ();
    CloseInputWindow();

    return;
    case IRV_DISPLAY_CANDWORDS:
    bShowNext = bShowPrev = False;
    if (bIsInLegend) {
        if (iCurrentLegendCandPage > 0)
        bShowPrev = True;
        if (iCurrentLegendCandPage < iLegendCandPageCount)
        bShowNext = True;
    }
    else {
        if (iCurrentCandPage > 0)
        bShowPrev = True;
        if (iCurrentCandPage < iCandPageCount)
        bShowNext = True;
    }

    DisplayInputWindow ();
    if (!bUseDBus) {
        DrawInputWindow ();
    }

    break;
    case IRV_DISPLAY_LAST:
    bShowNext = bShowPrev = False;
    SetMessageCount(&messageUp, 0);
    AddMessageAtLast(&messageUp, MSG_INPUT, "%c", strCodeInput[0]);
    SetMessageCount(&messageDown, 0);
    AddMessageAtLast(&messageDown, MSG_TIPS, strStringGet);
    DisplayInputWindow ();

    break;
    case IRV_DISPLAY_MESSAGE:
    bShowNext = False;
    bShowPrev = False;

    DisplayInputWindow ();
    if (!bUseDBus) {
        DrawInputWindow ();
    }

    break;
    case IRV_GET_LEGEND:
    SendHZtoClient (call_data, strStringGet);
    iHZInputed += (int) (utf8_strlen (strStringGet));	//粗略统计字数
    if (iLegendCandWordCount) {
        bShowNext = bShowPrev = False;
        if (iCurrentLegendCandPage > 0)
        bShowPrev = True;
        if (iCurrentLegendCandPage < iLegendCandPageCount)
        bShowNext = True;
        bLastIsNumber = False;
        iCodeInputCount = 0;
        DisplayInputWindow ();
        if (!bUseDBus) {
        DrawInputWindow ();
        }
    }
    else {
        ResetInput ();
        CloseInputWindow();
    }

    break;
    case IRV_GET_CANDWORDS:
    SendHZtoClient (call_data, strStringGet);
    bLastIsNumber = False;
    if (bPhraseTips && im[gs.iIMIndex].PhraseTips && !bVK)
        DoPhraseTips ();
    iHZInputed += (int) (utf8_strlen (strStringGet));	

    if (bVK || (!messageDown.msgCount && (!bPhraseTips || (bPhraseTips && !lastIsSingleHZ))))
        CloseInputWindow();
    else {
        DisplayInputWindow ();
        if (!bUseDBus) {
        DrawInputWindow ();
        }
    }

    ResetInput ();
    lastIsSingleHZ = 0;
    break;
    case IRV_ENG:
    //如果处于中文标点模式，应该将其中的标点转换为全角
    if (fcitxProfile.bChnPunc && bConvertPunc)
        ConvertPunc ();
    case IRV_PUNC:
    iHZInputed += (int) (utf8_strlen (strStringGet));	//粗略统计字数
    ResetInput ();
    if (!messageDown.msgCount)
        CloseInputWindow();
    case IRV_GET_CANDWORDS_NEXT:
    SendHZtoClient (call_data, strStringGet);
    bLastIsNumber = False;
    lastIsSingleHZ = 0;

    if (retVal == IRV_GET_CANDWORDS_NEXT || lastIsSingleHZ == -1) {
        iHZInputed += (int) (utf8_strlen (strStringGet));	//粗略统计字数
        DisplayInputWindow ();
    }

    break;
    default:
    ;
    }

    //计算打字速度的功能
    if (retVal == IRV_DISPLAY_MESSAGE || retVal == IRV_DISPLAY_CANDWORDS || retVal == IRV_PUNC) {
    if (!bStartRecordType) {
        bStartRecordType = True;
        timeStart = time (NULL);
    }
    }
}

Bool IsHotKey (int iKey, HOTKEYS * hotkey)
{
    if (iKey == hotkey[0] || iKey == hotkey[1])
    return True;
    return False;
}

INPUT_RETURN_VALUE ChangeCorner (void)
{
    ResetInput ();
    ResetInputWindow ();

    fcitxProfile.bCorner = !fcitxProfile.bCorner;

    SwitchIM(gs.iIMIndex);
    
    if (!bUseDBus) {
    DrawMainWindow ();
        CloseInputWindow();
    }
#ifdef _ENABLE_DBUS
    else
    updateProperty(&corner_prop);
#endif

    SaveProfile ();

    return IRV_DO_NOTHING;
}

INPUT_RETURN_VALUE ChangePunc (void)
{
    fcitxProfile.bChnPunc = !fcitxProfile.bChnPunc;
    
    if (!bUseDBus) 
        DrawMainWindow ();
    
    SaveProfile ();

#ifdef _ENABLE_DBUS
    if (bUseDBus)
    updateProperty(&punc_prop);
#endif

    return IRV_DO_NOTHING;
}

INPUT_RETURN_VALUE ChangeGBKT (void)
{
    fcitxProfile.bUseGBKT = !fcitxProfile.bUseGBKT;
    ResetInput ();
    ResetInputWindow ();

    if (!bUseDBus) {
    DrawMainWindow ();
        CloseInputWindow();
    }

    SaveProfile ();

#ifdef _ENABLE_DBUS
    if (bUseDBus)
    updateProperty(&gbkt_prop);
#endif

    return IRV_CLEAN;
}

INPUT_RETURN_VALUE ChangeLegend (void)
{
    fcitxProfile.bUseLegend = !fcitxProfile.bUseLegend;
    ResetInput ();
    
    if (!bUseDBus) {
    ResetInputWindow ();
    
    DrawMainWindow ();
        CloseInputWindow();
    }

    SaveProfile ();

#ifdef _ENABLE_DBUS
    if (bUseDBus)
    updateProperty(&legend_prop);
#endif
    return IRV_CLEAN;
}

INPUT_RETURN_VALUE ChangeTrack (void)
{
    fcitxProfile.bTrackCursor = !fcitxProfile.bTrackCursor;
    SaveProfile ();

    return IRV_DO_NOTHING;;
}

void ChangeLock (void)
{
    fcitxProfile.bLocked = !fcitxProfile.bLocked;
    
    if (!bUseDBus)
    DrawMainWindow ();

    SaveProfile ();
}

#ifdef _ENABLE_RECORDING
void ChangeRecording (void)
{
    fcitxProfile.bRecording =! fcitxProfile.bRecording;
    ResetInput ();
    ResetInputWindow ();
    CloseInputWindow();

    CloseRecording();
    if ( fcitxProfile.bRecording )
        OpenRecording(True);

    SaveProfile ();
}

void ResetRecording (void)
{
    if (fpRecord) {
        fclose(fpRecord);
        fpRecord = NULL;
        bWrittenRecord = False;
    }

    if ( fcitxProfile.bRecording )
        OpenRecording(False);
}
#endif

void SwitchIM (INT8 index)
{
    INT8        iLastIM;
    char	*str;

    if (index != (INT8) - 2 && bVK)
    return;

    iLastIM = (gs.iIMIndex >= iIMCount) ? (iIMCount - 1) : gs.iIMIndex;
    if (index == (INT8) - 1) {
    if (gs.iIMIndex == (iIMCount - 1))
        gs.iIMIndex = 0;
    else
        gs.iIMIndex++;
    }
    else if (index != (INT8) - 2) {
    if (index >= iIMCount)
        gs.iIMIndex = iIMCount - 1;
    else {
        gs.iIMIndex = index;
    }
    }

    if (bVK)
        str = vks[iCurrentVK].strName;
    else if (fcitxProfile.bCorner)
    str = strFullCorner;
    else
    str = im[gs.iIMIndex].strName;

    if (!bUseDBus) {
	XResizeWindow (dpy, mainWindow, skin_config.skin_main_bar.mbbg_img.width, skin_config.skin_main_bar.mbbg_img.height);
    DrawMainWindow ();
    }

    if (index != (INT8) - 2) {
    if (im[iLastIM].Save)
        im[iLastIM].Save();
    if (im[gs.iIMIndex].Init)
        im[gs.iIMIndex].Init ();
    }

    ResetInput ();
    CloseInputWindow();

    SaveProfile ();

#ifdef _ENABLE_DBUS
    if (bUseDBus) {
    
    if ((index == (INT8)-2) || (index == (INT8)-2)) {
        strcpy(logo_prop.label, "Fcitx");
        iState = IS_ENG;
    }
    else {
        int iIndex = ConnectIDGetState(connect_id);
        
        if (iIndex == IS_CHN) {
        strcpy(logo_prop.label, im[gs.iIMIndex].strName);
        iState = IS_CHN;
        }
    }
    
    updateProperty(&logo_prop);
    updateProperty(&state_prop);
    }
#endif
}

void SelectIM(int imidx)
{
//	int i=0;
	INT8        iLastIM;
	gs.iIMIndex=imidx;

    iLastIM = (gs.iIMIndex >= iIMCount) ? (iIMCount - 1) : gs.iIMIndex;

	if (im[iLastIM].Save)
	    im[iLastIM].Save();
	if (im[gs.iIMIndex].Init)
	    im[gs.iIMIndex].Init ();
	ResetInput ();
	DrawMainWindow ();
/*
	while(1)
	{
		if( iIMIndex == imidx )
			break;
		SwitchIM(-1);

		i++;
		if(i >20)
			break;
	}*/
	//printf("im[%d]:%s\n",iIMIndex,im[iIMIndex].strName);
}

void DoPhraseTips (void)
{
    if (!bPhraseTips)
    return;

    if (im[gs.iIMIndex].PhraseTips ())
    lastIsSingleHZ = -1;
    else
    lastIsSingleHZ = 0;
}

void RegisterNewIM (char *strName, char *strIconName, void (*ResetIM) (void),
            INPUT_RETURN_VALUE (*DoInput) (int), INPUT_RETURN_VALUE (*GetCandWords) (SEARCH_MODE), char *(*GetCandWord) (int), char *(*GetLegendCandWord) (int), Bool (*PhraseTips) (void), void (*Init) (void), void (*Save) (void))
{
#ifdef _DEBUG
    printf ("REGISTER %s\n", strName);
#endif
    strcpy (im[iIMCount].strName, strName);
    strcpy (im[iIMCount].strIconName, strIconName);
    im[iIMCount].ResetIM = ResetIM;
    im[iIMCount].DoInput = DoInput;
    im[iIMCount].GetCandWords = GetCandWords;
    im[iIMCount].GetCandWord = GetCandWord;
    im[iIMCount].GetLegendCandWord = GetLegendCandWord;
    im[iIMCount].PhraseTips = PhraseTips;
    im[iIMCount].Init = Init;
    im[iIMCount].Save = Save;

    iIMCount++;
}

Bool IsIM (char *strName)
{
    if (strstr (im[gs.iIMIndex].strName, strName))
    return True;

    return False;
}

void SaveIM (void)
{
    int i = 0;
    for (i = 0; i < iIMCount; i++)
    {
        if (im[i].Save)
            im[i].Save();
    }
}

void SetIM (void)
{
    INT8            i, j, k=0, l;
    Bool	    bFlag[INPUT_METHODS];

    if (im)
    free (im);

    if (inputMethods[IM_TABLE])
    LoadTableInfo ();

    iIMCount = tbl.iTableCount;
    if (inputMethods[IM_PY])
    iIMCount++;
    if (inputMethods[IM_SP])
    iIMCount++;
    if (inputMethods[IM_QW])
    iIMCount++;

    iIMCount += EIM_MAX;
    if (!iIMCount)
    iIMCount = 1;

    im = (IM *) malloc (sizeof (IM) * iIMCount);
    iIMCount = 0;

    /* 对输入法的次序进行添加 */
    for ( i = 0; i < INPUT_METHODS; i++ )
        bFlag[i] = False;

    for ( i = 0; i < INPUT_METHODS; i++ ) {
    l = 0;
    for ( j = 0; j < INPUT_METHODS; j++ ) {
        if ( !bFlag[j] ) {
            k = j;
            l = 1;
        }
        }

        l = 0;
    for ( j = (INPUT_METHODS-1); j>=0; j-- ) {
        if ( (inputMethods[k] >= inputMethods[j]) && !bFlag[j] ) {
            k = j;
            l = 1;
        }
    }

    if ( !l )
        break;

    bFlag[k] = True;
    if ( inputMethods[k]>0 ) {
        switch (k) {
        case IM_PY:
        RegisterNewIM (strNameOfPinyin, strIconNameOfPinyin, ResetPYStatus, DoPYInput, PYGetCandWords, PYGetCandWord, PYGetLegendCandWord, NULL, PYInit, SavePY);
        break;
        case IM_SP:
        RegisterNewIM (strNameOfShuangpin, strIconNameOfShuangpin, ResetPYStatus, DoPYInput, PYGetCandWords, PYGetCandWord, PYGetLegendCandWord, NULL, SPInit, SavePY);
        break;
        case IM_QW:
        RegisterNewIM (strNameOfQuwei, strIconNameOfQuwei, NULL, DoQWInput, QWGetCandWords, QWGetCandWord, NULL, NULL, NULL, NULL);
        break;
        case IM_TABLE:
        for (l = 0; l < tbl.iTableCount; l++) {
            TABLE* table = (TABLE*) utarray_eltptr(tbl.table, l);
            RegisterNewIM (table->strName, table->strIconName, TableResetStatus, DoTableInput, TableGetCandWords, TableGetCandWord, TableGetLegendCandWord, TablePhraseTips, TableInit, FreeTableIM);
            table->iIMIndex = iIMCount - 1;
        }
        default:
            break;
        }
    }
    }

    if ( (!inputMethods[IM_SP] && (!inputMethods[IM_TABLE] || !tbl.iTableCount)) && !iIMCount )	 //至少应该有一种输入法
    RegisterNewIM (strNameOfPinyin, strIconNameOfPinyin, ResetPYStatus, DoPYInput, PYGetCandWords, PYGetCandWord, PYGetLegendCandWord, NULL, PYInit, NULL);

    if (strExternIM[0] && strExternIM[1])
    LoadExtraIM(strExternIM);

    SwitchIM (gs.iIMIndex);
}
