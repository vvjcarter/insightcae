#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

#include "iqmatrixparameter.h"
#include "iqparametersetmodel.h"
#include "base/table.h"

defineType(IQMatrixParameter);
addToFactoryTable(IQParameter, IQMatrixParameter);

IQMatrixParameter::IQMatrixParameter
(
    QObject* parent,
    const QString& name,
    insight::Parameter& parameter,
    const insight::ParameterSet& defaultParameterSet
)
  : IQParameter(parent, name, parameter, defaultParameterSet)
{
}


QString IQMatrixParameter::valueText() const
{
  const auto& p = dynamic_cast<const insight::MatrixParameter&>(parameter());

  return QString("matrix %1x%2").arg( p().n_rows ).arg( p().n_cols );
}

QString mat2Str(const arma::mat& m)
{
  std::ostringstream oss;
  for (arma::uword i=0; i<m.n_rows; i++)
  {
    for (arma::uword j=0; j<m.n_cols; j++)
    {
      oss<<m(i,j);
      if (j!=m.n_cols-1) oss<<" ";
    }
    if (i!=m.n_rows-1) oss<<";";
  }
  return QString(oss.str().c_str());
}

QVBoxLayout* IQMatrixParameter::populateEditControls(IQParameterSetModel* model, const QModelIndex &index, QWidget* editControlsContainer)
{
  const auto& p = dynamic_cast<const insight::MatrixParameter&>(parameter());

  auto* layout = IQParameter::populateEditControls(model, index, editControlsContainer);

  QHBoxLayout *layout2=new QHBoxLayout(editControlsContainer);
  QLabel *promptLabel = new QLabel("Value:", editControlsContainer);
  layout2->addWidget(promptLabel);
  auto *lineEdit = new QLineEdit(editControlsContainer);
//  connect(le_, &QLineEdit::destroyed, this, &MatrixParameterWrapper::onDestruction);
  lineEdit->setText(mat2Str(p()));
  layout2->addWidget(lineEdit);
  auto *dlgBtn_=new QPushButton("...", editControlsContainer);
  layout2->addWidget(dlgBtn_);
  layout->addLayout(layout2);

  QPushButton* apply=new QPushButton("&Apply", editControlsContainer);
  layout->addWidget(apply);

  layout->addStretch();

  auto applyFunction = [=]()
  {
    auto&p = dynamic_cast<insight::MatrixParameter&>(model->parameterRef(index));
    p()=arma::mat(lineEdit->text().toStdString());
    model->notifyParameterChange(index);
  };

  connect(lineEdit, &QLineEdit::returnPressed, applyFunction);
  connect(apply, &QPushButton::pressed, applyFunction);

  connect(dlgBtn_, &QPushButton::clicked, dlgBtn_,
          [lineEdit, editControlsContainer, applyFunction]()
          {
              QString fn = QFileDialog::getOpenFileName(
                    editControlsContainer,
                    "Select file",
                    QString(),
                    "CSV file (*.csv)" );
              if (!fn.isEmpty())
              {
                  std::ifstream f(fn.toStdString());
                  insight::Table tab(f);
                  insight::assertion(
                              tab.nCols()==2,
                              str(boost::format("A table with 2 columns was expected! Got: %dx%d matrix.") % tab.nRows() % tab.nCols()) );
                  lineEdit->setText(mat2Str(tab.xy(0, 1)));
                  applyFunction();
              }
          }
  );

  return layout;
}
