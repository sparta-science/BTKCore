/* 
 * The Biomechanical ToolKit
 * Copyright (c) 2009-2010, Arnaud Barré
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name(s) of the copyright holders nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "Acquisition.h"

#include <Qt>

Acquisition::Acquisition(QObject* parent)
: QObject(parent), m_Filename(), mp_Acquisition(), m_Points()
{
  this->m_FirstFrame = 0;
  this->m_LastFrame = 0;
  this->mp_ROI[0] = this->m_FirstFrame;
  this->mp_ROI[1] = this->m_LastFrame;
  this->m_PointFrequency = 0.0;
  this->m_LastEventId = -1;
};

Acquisition::~Acquisition()
{
  this->clear();
};

void Acquisition::init(const QString& filename, btk::Acquisition::Pointer acquisition)
{
  this->clear();
  //qDebug("Points count: %i", this->m_Points.count());
  this->m_Filename = filename;
  this->mp_Acquisition = acquisition;
  this->m_FirstFrame = acquisition->GetFirstFrame();
  this->m_LastFrame = acquisition->GetLastFrame();
  this->mp_ROI[0] = this->m_FirstFrame;
  this->mp_ROI[1] = this->m_LastFrame;
  this->m_PointFrequency = acquisition->GetPointFrequency();
  // Points
  int inc = 0;
  for (btk::Acquisition::PointIterator it = this->mp_Acquisition->BeginPoint() ; it != this->mp_Acquisition->EndPoint() ; ++it)
  {
    Point* p = new Point();
    p->label = QString::fromStdString((*it)->GetLabel());
    p->description = QString::fromStdString((*it)->GetDescription());
    if ((*it)->GetType() == btk::Point::Marker)
    {
      p->type = Marker;
      p->radius = 8.0;
      p->color = Qt::white;
    }
    else
    {
      if ((*it)->GetType() == btk::Point::Angle)
        p->type = Angle;
      else if ((*it)->GetType() == btk::Point::Force)
        p->type = Force;
      else if ((*it)->GetType() == btk::Point::Moment)
        p->type = Moment;
      else if ((*it)->GetType() == btk::Point::Power)
        p->type = Power;
      else if ((*it)->GetType() == btk::Point::Scalar)
        p->type = Scalar;
      p->radius = -1.0;
      p->color = QColor::Invalid;
    }
    this->m_Points.insert(inc, p);
    ++inc;
  }
  // Analog
  inc = 0;
  for (btk::Acquisition::AnalogIterator it = this->mp_Acquisition->BeginAnalog() ; it != this->mp_Acquisition->EndAnalog() ; ++it)
  {
    Analog* a = new Analog();
    a->label = QString::fromStdString((*it)->GetLabel());
    a->description = QString::fromStdString((*it)->GetDescription());
    a->unit = QString::fromStdString((*it)->GetUnit());
    switch((*it)->GetGain())
    {
    case btk::Analog::Unknown:
      a->gain = Unknown;
      break;
    case btk::Analog::PlusMinus10:
      a->gain = PlusMinus10;
      break;
    case btk::Analog::PlusMinus5:
      a->gain = PlusMinus5;
      break;
    case btk::Analog::PlusMinus2Dot5:
      a->gain = PlusMinus2Dot5;
      break;
    case btk::Analog::PlusMinus1Dot25:
      a->gain = PlusMinus1Dot25;
      break;
    case btk::Analog::PlusMinus1:
      a->gain = PlusMinus1;
      break;
    }
    a->offset = (*it)->GetOffset();
    a->scale = (*it)->GetScale();
    this->m_Analogs.insert(inc, a);
    ++inc;
  }
  // Event
  inc = 0;
  for (btk::Acquisition::EventIterator it = this->mp_Acquisition->BeginEvent() ; it != this->mp_Acquisition->EndEvent() ; ++it)
  {
    Event* e = new Event();
    e->label = QString::fromStdString((*it)->GetLabel());;
    e->description = QString::fromStdString((*it)->GetDescription());
    e->context = QString::fromStdString((*it)->GetContext());
    e->subject = QString::fromStdString((*it)->GetSubject());
    e->time = (*it)->GetTime();
    e->frame = (*it)->GetFrame();
    e->iconId = (*it)->GetId();;
    this->m_Events.insert(inc, e);
    ++inc;
  }
  this->m_LastEventId = inc - 1;
};

void Acquisition::clear()
{
  for (QMap<int,Point*>::iterator it = this->m_Points.begin() ; it != this->m_Points.end() ; ++it)
    delete *it;
  this->m_Points.clear();
  for (QMap<int,Analog*>::iterator it = this->m_Analogs.begin() ; it != this->m_Analogs.end() ; ++it)
    delete *it;
  this->m_Analogs.clear();
  for (QMap<int,Event*>::iterator it = this->m_Events.begin() ; it != this->m_Events.end() ; ++it)
    delete *it;
  this->m_Events.clear();
  this->m_LastEventId = -1;
}

void Acquisition::setRegionOfInterest(int lb, int rb)
{
  this->mp_ROI[0] = lb;
  this->mp_ROI[1] = rb;
  emit regionOfInterestChanged(lb, rb);
};

void Acquisition::setPointLabel(int id, const QString& label)
{
  QMap<int,Point*>::iterator it = this->m_Points.find(id);
  if (it != this->m_Points.end())
  {
    (*it)->label = label;
    emit pointLabelChanged(id, label);
  }
};

void Acquisition::setPointsDescription(const QVector<int>& ids, const QVector<QString>& descs)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Point*>::iterator it = this->m_Points.find(ids[i]);
    if (it != this->m_Points.end())
      (*it)->description = descs[i];
  }
  emit pointsDescriptionChanged(ids, descs);
};

void Acquisition::setPointType(int id, PointType p)
{
  QMap<int,Point*>::iterator it = this->m_Points.find(id);
  if (it != this->m_Points.end())
  {
    (*it)->type = p;
    emit pointTypeChanged(id, p);
  }
};

void Acquisition::setMarkersRadius(const QVector<int>& ids, const QVector<double>& radii)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Point*>::iterator it = this->m_Points.find(ids[i]);
    if (it != this->m_Points.end())
      (*it)->radius = radii[i];
  }
  emit markersRadiusChanged(ids, radii);
};

void Acquisition::setMarkerColor(int id, const QColor& color)
{
  QMap<int,Point*>::iterator it = this->m_Points.find(id);
  if (it != this->m_Points.end())
  {
    (*it)->color = color;
    emit markerColorChanged(id, color);
  }
};

void Acquisition::setMarkersColor(const QVector<int>& ids, const QVector<QColor>& colors)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Point*>::iterator it = this->m_Points.find(ids[i]);
    if (it != this->m_Points.end())
      (*it)->color = colors[i];
  }
  emit markersColorChanged(ids, colors);
};

QList<Point*> Acquisition::takePoints(const QList<int>& ids)
{
  QList<Point*> points;
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Point*>::iterator it = this->m_Points.find(ids[i]);
    if (it != this->m_Points.end())
    {
      points.push_back(*it);
      this->m_Points.erase(it);
    }
  }
  emit pointsRemoved(ids, points);
  return points;
};

void Acquisition::insertPoints(const QList<int>& ids, const QList<Point*> points)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Point*>::iterator it = this->m_Points.find(ids[i]);
    if (it == this->m_Points.end())
      this->m_Points.insert(ids[i], points[i]);
    else
      qDebug("A point with the given ID already exists");
  }
  emit pointsInserted(ids, points);
};

int Acquisition::findPointIdFromLabel(const QString& label) const
{
  for (QMap<int,Point*>::const_iterator it = this->m_Points.begin() ; it != this->m_Points.end() ; ++it)
  {
    if ((*it)->label.compare(label) == 0)
      return it.key();
  }
  return -1;
};

void Acquisition::setAnalogLabel(int id, const QString& label)
{
  QMap<int,Analog*>::iterator it = this->m_Analogs.find(id);
  if (it != this->m_Analogs.end())
  {
    (*it)->label = label;
    emit analogLabelChanged(id, label);
  }
};

void Acquisition::setAnalogsDescription(const QVector<int>& ids, const QVector<QString>& descs)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Analog*>::iterator it = this->m_Analogs.find(ids[i]);
    if (it != this->m_Analogs.end())
      (*it)->description = descs[i];
  }
  emit analogsDescriptionChanged(ids, descs);
};

void Acquisition::setAnalogsUnit(const QVector<int>& ids, const QVector<QString>& units)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Analog*>::iterator it = this->m_Analogs.find(ids[i]);
    if (it != this->m_Analogs.end())
      (*it)->unit = units[i];
  }
  emit analogsUnitChanged(ids, units);
};

void Acquisition::setAnalogsGain(const QVector<int>& ids, const QVector<AnalogGain>& gains)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Analog*>::iterator it = this->m_Analogs.find(ids[i]);
    if (it != this->m_Analogs.end())
      (*it)->gain = gains[i];
  }
  emit analogsGainChanged(ids, gains);
};

void Acquisition::setAnalogsOffset(const QVector<int>& ids, const QVector<int>& offsets)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Analog*>::iterator it = this->m_Analogs.find(ids[i]);
    if (it != this->m_Analogs.end())
      (*it)->offset = offsets[i];
  }
  emit analogsOffsetChanged(ids, offsets);
};

void Acquisition::setAnalogsScale(const QVector<int>& ids, const QVector<double>& scales)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Analog*>::iterator it = this->m_Analogs.find(ids[i]);
    if (it != this->m_Analogs.end())
      (*it)->scale = scales[i];
  }
  emit analogsScaleChanged(ids, scales);
};

QList<Analog*> Acquisition::takeAnalogs(const QList<int>& ids)
{
  QList<Analog*> analogs;
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Analog*>::iterator it = this->m_Analogs.find(ids[i]);
    if (it != this->m_Analogs.end())
    {
      analogs.push_back(*it);
      this->m_Analogs.erase(it);
    }
  }
  emit analogsRemoved(ids, analogs);
  return analogs;
};

void Acquisition::insertAnalogs(const QList<int>& ids, const QList<Analog*> analogs)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Analog*>::iterator it = this->m_Analogs.find(ids[i]);
    if (it == this->m_Analogs.end())
      this->m_Analogs.insert(ids[i], analogs[i]);
    else
      qDebug("An analog channel with the given ID already exists");
  }
  emit analogsInserted(ids, analogs);
};

const Event* Acquisition::eventAt(int id) const
{
  QMap<int,Event*>::const_iterator it = this->m_Events.find(id);
  if (it != this->m_Events.end())
    return *it;
  return 0;
};

QList<Event*> Acquisition::takeEvents(const QList<int>& ids)
{
  QList<Event*> events;
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Event*>::iterator it = this->m_Events.find(ids[i]);
    if (it != this->m_Events.end())
    {
      events.push_back(*it);
      this->m_Events.erase(it);
    }
  }
  emit eventsRemoved(ids, events);
  return events;
};

void Acquisition::insertEvents(const QList<int>& ids, const QList<Event*> events)
{
  for (int i = 0 ; i < ids.count() ; ++i)
  {
    QMap<int,Event*>::iterator it = this->m_Events.find(ids[i]);
    if (it == this->m_Events.end())
      this->m_Events.insert(ids[i], events[i]);
    else
      qDebug("An event with the given ID already exists");
    if (ids[i] > this->m_LastEventId)
      this->m_LastEventId = ids[i];
  }
  emit eventsInserted(ids, events);
};

int Acquisition::generateNewEventId()
{
  this->m_LastEventId += 1;
  return this->m_LastEventId;
};