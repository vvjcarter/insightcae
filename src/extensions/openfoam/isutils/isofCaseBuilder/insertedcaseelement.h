#ifndef INSERTEDCASEELEMENT_H
#define INSERTEDCASEELEMENT_H

#include <QObject>
#include <QListWidget>
#include <QListWidgetItem>

#include <string>

#ifndef Q_MOC_RUN
#include "openfoam/openfoamcase.h"
#include "openfoam/caseelements/boundarycondition.h"
#endif


namespace insight {
class Multi_CAD_ParameterSet_Visualizer;
}

class CaseElementData : public QObject
{
  Q_OBJECT

protected:
  std::string type_name_;
  insight::ParameterSet curp_;

  insight::Multi_CAD_ParameterSet_Visualizer* mv_;
  insight::ParameterSet_VisualizerPtr viz_;

public:
  CaseElementData(
      const std::string& type_name,
      insight::Multi_CAD_ParameterSet_Visualizer* d,
      QObject* parent=nullptr );

  ~CaseElementData();

  inline insight::ParameterSet& parameters() { return curp_; }
  virtual const insight::ParameterSet defaultParameters() const;
  inline const insight::ParameterSet& parameters() const { return curp_; }
  inline const std::string& type_name() const { return type_name_; }

  insight::ParameterSet_VisualizerPtr visualizer();
  insight::Multi_CAD_ParameterSet_Visualizer* multiVisualizer() const;
  void updateVisualization();
};



class InsertedCaseElement
: public CaseElementData
{

public:
    InsertedCaseElement(
        const std::string& type_name,
        insight::Multi_CAD_ParameterSet_Visualizer* d,
        QObject* parent=nullptr );

    insight::OpenFOAMCaseElement* createElement(insight::OpenFOAMCase& c) const;
    void insertElement(insight::OpenFOAMCase& ofc) const;

};



#endif // INSERTEDCASEELEMENT_H
