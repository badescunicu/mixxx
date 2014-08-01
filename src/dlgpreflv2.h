#ifndef DLGPREFLV2_H
#define DLGPREFLV2_H

#include <QWidget>

#include "ui_dlgpreflv2dlg.h"
#include "configobject.h"
#include "preferences/dlgpreferencepage.h"
#include "effects/lv2/lv2backend.h"

class DlgPrefLV2 : public DlgPreferencePage, public Ui::DlgPrefLV2Dlg  {
    Q_OBJECT
  public:
    DlgPrefLV2(QWidget *parent, LV2Backend* lv2Backend,
               ConfigObject<ConfigValue>* _config);
    virtual ~DlgPrefLV2();
};

#endif