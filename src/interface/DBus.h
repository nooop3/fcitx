#ifdef _ENABLE_DBUS

#include <dbus/dbus.h>
#include "core/main.h"
#include "ui/InputWindow.h"
#include "core/ime.h"
#include "core/xim.h"

extern IM *im;

extern INT8 iState;
extern Bool bVK;
extern Bool bUseTable;
extern Bool bUseQW;
extern Bool bUseSP;
extern Bool bUseAA;
extern Bool bUseMatchingKey;
extern Bool bUsePinyin;
extern Bool bUseBold;

typedef struct Property_ {
    char *key;
    char *label;
    char *icon;
    char *tip;
} Property;

extern DBusConnection *conn;

Bool InitDBus();

void KIMExecDialog(char *prop);
void KIMExecMenu(char *props[],int n);
void KIMRegisterProperties(char *props[],int n);
void KIMUpdateProperty(char *prop);
void KIMRemoveProperty(char *prop);
void KIMEnable(Bool toEnable);
void KIMShowAux(Bool toShow);
void KIMShowPreedit(Bool toShow);
void KIMShowLookupTable(Bool toShow);
void KIMUpdateLookupTable(char *labels[], int nLabel, char *texts[], int nText, Bool has_prev, Bool has_next);
void KIMUpdatePreeditCaret(int position);
void KIMUpdatePreeditText(char *text);
void KIMUpdateAux(char *text);
void KIMUpdateSpotLocation(int x,int y);
void KIMUpdateScreen(int id);

void updateMessages();

void registerProperties();
void updateProperty(Property *prop);
void triggerProperty(char *propKey);

char* property2string(Property *prop);

void updatePropertyByConnectID(CARD16 connect_id);

void DBusLoop(void *val);
void MyDBusEventHandler();

#endif 
