#ifndef IQARRAYPARAMETER_H
#define IQARRAYPARAMETER_H

#include "toolkit_gui_export.h"


#include <iqparameter.h>

#include "base/parameters/arrayparameter.h"

class TOOLKIT_GUI_EXPORT IQArrayParameter : public IQParameter
{
public:
  declareType(insight::ArrayParameter::typeName_());

  IQArrayParameter
  (
      QObject* parent,
      const QString& name,
      insight::Parameter& parameter,
      const insight::ParameterSet& defaultParameterSet
  );

  QString valueText() const override;


  QVBoxLayout* populateEditControls(IQParameterSetModel* model, const QModelIndex &index, QWidget* editControlsContainer) override;

};

#endif // IQARRAYPARAMETER_H
