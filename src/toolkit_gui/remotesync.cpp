
#include "remotesync.h"


namespace insight {


RunSyncToRemote::RunSyncToRemote(insight::RemoteExecutionConfig& rec)
  : rec_(rec)
{}

void RunSyncToRemote::run()
{
  rec_.syncToRemote
      (
        {},
        [&](int progress, const std::string& progress_text)
        {
          Q_EMIT progressValueChanged(progress);
          Q_EMIT progressTextChanged(QString(progress_text.c_str()));
        }
      );
  emit transferFinished();
}




RunSyncToLocal::RunSyncToLocal(insight::RemoteExecutionConfig& rec)
  : rec_(rec)
{}

void RunSyncToLocal::run()
{
  rec_.syncToLocal
      (
        false,
        {},
        [&](int progress, const std::string& progress_text)
        {
          Q_EMIT progressValueChanged(progress);
          Q_EMIT progressTextChanged(QString(progress_text.c_str()));
        }
      );
  Q_EMIT transferFinished();
}

}
