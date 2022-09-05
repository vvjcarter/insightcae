
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

#include "vtkProperty.h"
#include "vtkTransform.h"
#include "vtkPolyDataMapper.h"

#include "iqspatialtransformationparameter.h"
#include "iqparametersetmodel.h"
#include "iqcadmodel3dviewer.h"
#include "iqcadtransformationcommand.h"
#include "ivtkoccshape.h"

#include "cadfeature.h"


defineType(IQSpatialTransformationParameter);
addToFactoryTable(IQParameter, IQSpatialTransformationParameter);




IQSpatialTransformationParameter::IQSpatialTransformationParameter
(
    QObject* parent,
    const QString& name,
    insight::Parameter& parameter,
    const insight::ParameterSet& defaultParameterSet
)
  : IQParameter(parent, name, parameter, defaultParameterSet)
{
}




QString IQSpatialTransformationParameter::valueText() const
{
  const auto& p = dynamic_cast<const insight::SpatialTransformationParameter&>(parameter());

  if (p().isIdentityTransform())
      return QString("identity");
  else
      return QString("(transform)");
}





QVBoxLayout* IQSpatialTransformationParameter::populateEditControls(
        IQParameterSetModel* model,
        const QModelIndex &index,
        QWidget* editControlsContainer,
        IQCADModel3DViewer *viewer )
{
  const auto& p =
          dynamic_cast<const insight::SpatialTransformationParameter&>(
              parameter() );

  auto* layout = IQParameter::populateEditControls(model, index, editControlsContainer, viewer);

  QLineEdit *translateLE, *rpyLE, *scaleLE;

  {
      QHBoxLayout *layout2=new QHBoxLayout;
      QLabel *translateLabel = new QLabel("Translation:", editControlsContainer);
      layout2->addWidget(translateLabel);
      translateLE = new QLineEdit(editControlsContainer);
//      translateLE->setText(mat2Str(p().translate()));
      layout2->addWidget(translateLE);
      layout->addLayout(layout2);
  }

  {
      QHBoxLayout *layout2=new QHBoxLayout;
      QLabel *rpyLabel = new QLabel("Roll Pitch Yaw:", editControlsContainer);
      layout2->addWidget(rpyLabel);
      rpyLE = new QLineEdit(editControlsContainer);
//      rpyLE->setText(mat2Str(p().rollPitchYaw()));
      layout2->addWidget(rpyLE);
      layout->addLayout(layout2);
  }

  {
      QHBoxLayout *layout2=new QHBoxLayout;
      QLabel *scaleLabel = new QLabel("Scale factor:", editControlsContainer);
      layout2->addWidget(scaleLabel);
      scaleLE = new QLineEdit(editControlsContainer);
//      scaleLE->setText(QString::number(p().scale()));
      layout2->addWidget(scaleLE);
      layout->addLayout(layout2);
  }

  auto setValuesToControls =
          [translateLE,rpyLE,scaleLE](const insight::SpatialTransformation& t)
  {
      translateLE->setText(mat2Str(t.translate()));
      rpyLE->setText(mat2Str(t.rollPitchYaw()));
      scaleLE->setText(QString::number(t.scale()));
  };

  setValuesToControls(p());

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
    auto&p =
            dynamic_cast<insight::SpatialTransformationParameter&>(
                model->parameterRef(index) );

    arma::mat m;

    insight::stringToValue(translateLE->text().toStdString(), m);
    p().setTranslation(m);

    insight::stringToValue(rpyLE->text().toStdString(), m);
    p().setRollPitchYaw(m);

    bool ok;
    p().setScale(scaleLE->text().toDouble(&ok));
    insight::assertion(ok, "invalid input for scale factor!");

    model->notifyParameterChange(index);
  };

  connect(translateLE, &QLineEdit::returnPressed, applyFunction);
  connect(rpyLE, &QLineEdit::returnPressed, applyFunction);
  connect(scaleLE, &QLineEdit::returnPressed, applyFunction);
  connect(apply, &QPushButton::pressed, applyFunction);

  if (viewer)
  {
    connect(dlgBtn_, &QPushButton::clicked, dlgBtn_,
          [this,translateLE,model,viewer,setValuesToControls,apply]()
          {
            if (insight::cad::FeaturePtr geom =
                    model->getGeometryToSpatialTransformationParameter(path()))
            {
                vtkNew<ivtkOCCShape> shape;
                shape->SetShape( geom->shape() );
                auto actor = vtkSmartPointer<vtkActor>::New();
                actor->SetMapper( vtkSmartPointer<vtkPolyDataMapper>::New() );
                actor->GetMapper()->SetInputConnection(shape->GetOutputPort());
                actor->GetProperty()->SetOpacity(0.7);
                actor->GetProperty()->SetColor(0.7, 0.3, 0.3);

//                if (vtkActor* actor = viewer->getActor(geom))
                const auto& p =
                        dynamic_cast<const insight::SpatialTransformationParameter&>(
                            parameter() );
                {
                    auto tini = p().toVTKTransform();

                  auto curMod =
                        new IQCADTransformationCommand(
                              actor, viewer->interactor(),
                              tini );

                  connect( translateLE, &QObject::destroyed,
                           curMod, &QObject::deleteLater );

                  connect( apply, &QPushButton::pressed,
                           curMod, &QObject::deleteLater );

                  connect( curMod, &IQCADTransformationCommand::dataChanged, this,
                           [this,curMod,setValuesToControls]()
                           {
                             setValuesToControls(curMod->getSpatialTransformation());
                           } );
                }
            }
          }
    );
  }

  return layout;
}
