#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "iqvectorparameter.h"
#include "iqparametersetmodel.h"

#include "iqcadmodel3dviewer.h"
#include "iqpointpickcommand.h"
#include "iqvectordirectioncommand.h"

defineType(IQVectorParameter);
addToFactoryTable(IQParameter, IQVectorParameter);

IQVectorParameter::IQVectorParameter
(
    QObject* parent,
    const QString& name,
    insight::Parameter& parameter,
    const insight::ParameterSet& defaultParameterSet
) : IQParameter(parent, name, parameter, defaultParameterSet)
{}


QString IQVectorParameter::valueText() const
{
  const auto& p = dynamic_cast<const insight::VectorParameter&>(parameter());

  return "["+QString::fromStdString(insight::valueToString(p()))+"]";
}



QVBoxLayout* IQVectorParameter::populateEditControls(
        IQParameterSetModel* model, const QModelIndex &index, QWidget* editControlsContainer,
        IQCADModel3DViewer *viewer)
{
  const auto& p= dynamic_cast<const insight::VectorParameter&>(parameter());

  auto* layout = IQParameter::populateEditControls(model, index, editControlsContainer, viewer);

  QHBoxLayout *layout2=new QHBoxLayout;
  QLabel *promptLabel = new QLabel("Value:", editControlsContainer);
  layout2->addWidget(promptLabel);
  auto *lineEdit=new QLineEdit(editControlsContainer);

  lineEdit->setText( QString::fromStdString(insight::valueToString(p())) );

  layout2->addWidget(lineEdit);
  layout->addLayout(layout2);

  QPushButton *dlgBtn_=nullptr;
  if (viewer)
  {
      dlgBtn_ = new QPushButton("...", editControlsContainer);
      layout->addWidget(dlgBtn_);
  }

  QPushButton* apply=new QPushButton("&Apply", editControlsContainer);
  layout->addWidget(apply);

  layout->addStretch();

  auto applyFunction = [=]()
  {
    auto& p = dynamic_cast<insight::VectorParameter&>(model->parameterRef(index));
    insight::stringToValue(lineEdit->text().toStdString(), p());
    model->notifyParameterChange(index);
  };

  connect(lineEdit, &QLineEdit::returnPressed, applyFunction);
  connect(apply, &QPushButton::pressed, applyFunction);

  if (viewer)
  {
    connect(dlgBtn_, &QPushButton::clicked, dlgBtn_,
          [this,model,viewer,apply,lineEdit]()
          {
            const auto& p =
                    dynamic_cast<const insight::VectorParameter&>(
                        parameter() );
            if (auto bp =
                    model->getVectorBasePoint(path()))
            {
                auto curMod =
                      new IQVectorDirectionCommand(
                            viewer->interactor(),
                            (*bp), p() );

                connect( apply, &QPushButton::pressed,
                         curMod, &QObject::deleteLater );

                connect( curMod, &IQVectorDirectionCommand::dataChanged, curMod,
                         [this,curMod,lineEdit]()
                         {
                           lineEdit->setText(
                                       QString::fromStdString(
                                           insight::valueToString(
                                               curMod->getVector()
                                               ) ) );
                         } );
            }
            else
            {
              auto curMod =
                    new IQPointPickCommand(
                          viewer->interactor(),
                          p() );

              connect( apply, &QPushButton::pressed,
                       curMod, &QObject::deleteLater );

              connect( curMod, &IQPointPickCommand::dataChanged, curMod,
                       [this,curMod,lineEdit]()
                       {
                         lineEdit->setText(
                                     QString::fromStdString(
                                         insight::valueToString(
                                             curMod->getPickedPosition()
                                             ) ) );
                         curMod->deleteLater();
                       } );
            }
          }
    );
  }

  return layout;
}
