/*
 * This file is part of Insight CAE, a workbench for Computer-Aided Engineering
 * Copyright (C) 2014  Hannes Kroeger <hannes@kroegeronline.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef INSIGHT_QCHART_H
#define INSIGHT_QCHART_H

#include "toolkit_gui_export.h"


#include "qimage.h"

namespace insight {

class TOOLKIT_GUI_EXPORT QChart
 : public QImage
{
    Q_OBJECT

public:
    declareType ( insight::Chart::typeName_() );

    QChart(QObject* parent, const QString& label, insight::ResultElementPtr rep);
};

} // namespace insight

#endif // INSIGHT_QCHART_H
