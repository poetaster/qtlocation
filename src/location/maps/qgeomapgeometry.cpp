/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qgeomapgeometry_p.h"

#include "qgeocameradata_p.h"
#include "qgeoprojection_p.h"
#include "qgeotilecache_p.h"
#include "qgeotilespec.h"

#include "qdoublevector2d_p.h"
#include "qdoublevector3d_p.h"

#include <Qt3D/qglscenenode.h>
#include <Qt3D/qglbuilder.h>
#include <Qt3D/qglmaterial.h>
#include <Qt3D/qgltexture2d.h>
#include <Qt3D/qgeometrydata.h>
#include <Qt3D/qglcamera.h>
#include <Qt3D/qglpainter.h>

#include <QHash>

#include <QPointF>

#include <cmath>

QT_BEGIN_NAMESPACE

class QGeoMapGeometryPrivate {
public:
    QGeoMapGeometryPrivate(QGeoMapGeometry *geometry);
    ~QGeoMapGeometryPrivate();

    QSize screenSize_;
    int tileSize_;
    QGeoCameraData cameraData_;
    QSet<QGeoTileSpec> visibleTiles_;

    QGLCamera *camera_;
    QGLSceneNode *sceneNode_;

    double scaleFactor_;

    int intZoomLevel_;
    int sideLength_;

    QHash<QGeoTileSpec, QGLSceneNode*> nodes_;
    QHash<QGeoTileSpec, QSharedPointer<QGeoTileTexture> > textures_;
    QList<QSharedPointer<QGeoTileTexture> > newUploads_;

    int minTileX_;
    int minTileY_;
    int maxTileX_;
    int maxTileY_;
    int tileZ_;
    int tileXWrapsBelow_;

    double mercatorCenterX_;
    double mercatorCenterY_;
    double mercatorWidth_;
    double mercatorHeight_;

    double screenOffsetX_;
    double screenOffsetY_;
    double screenWidth_;
    double screenHeight_;

    bool useVerticalLock_;
    bool verticalLock_;

    void addTile(const QGeoTileSpec &spec, QSharedPointer<QGeoTileTexture> texture);

    QDoubleVector2D screenPositionToMercator(const QPointF &pos) const;
    QPointF mercatorToScreenPosition(const QDoubleVector2D &mercator) const;

    void setVisibleTiles(const QSet<QGeoTileSpec> &tiles);
    void removeTiles(const QSet<QGeoTileSpec> &oldTiles);
    void updateTiles(const QSet<QGeoTileSpec> &tiles);
    QGLSceneNode *buildGeometry(const QGeoTileSpec &spec);
    void setTileBounds(const QSet<QGeoTileSpec> &tiles);
    void setupCamera();

    void paintGL(QGLPainter *painter);

private:
    QGeoMapGeometry *q_ptr;
    Q_DECLARE_PUBLIC(QGeoMapGeometry)
};

QGeoMapGeometry::QGeoMapGeometry()
    : QObject(),
      d_ptr(new QGeoMapGeometryPrivate(this)) {}

QGeoMapGeometry::~QGeoMapGeometry()
{
    delete d_ptr;
}

void QGeoMapGeometry::setUseVerticalLock(bool lock)
{
    Q_D(QGeoMapGeometry);
    d->useVerticalLock_ = lock;
}

void QGeoMapGeometry::setScreenSize(const QSize &size)
{
    Q_D(QGeoMapGeometry);
    d->screenSize_ = size;
}

void QGeoMapGeometry::setTileSize(int tileSize)
{
    Q_D(QGeoMapGeometry);
    d->tileSize_ = tileSize;
}

void QGeoMapGeometry::setCameraData(const QGeoCameraData &cameraData)
{
    Q_D(QGeoMapGeometry);
    d->cameraData_ = cameraData;
    d->intZoomLevel_ = static_cast<int>(floor(d->cameraData_.zoomLevel()));
    d->sideLength_ = 1 << d->intZoomLevel_;
}

void QGeoMapGeometry::setVisibleTiles(const QSet<QGeoTileSpec> &tiles)
{
    Q_D(QGeoMapGeometry);
    d->setVisibleTiles(tiles);
}

void QGeoMapGeometry::addTile(const QGeoTileSpec &spec, QSharedPointer<QGeoTileTexture> texture)
{
    Q_D(QGeoMapGeometry);
    d->addTile(spec, texture);
}

QDoubleVector2D QGeoMapGeometry::screenPositionToMercator(const QPointF &pos) const
{
    Q_D(const QGeoMapGeometry);
    return d->screenPositionToMercator(pos);
}

QPointF QGeoMapGeometry::mercatorToScreenPosition(const QDoubleVector2D &mercator) const
{
    Q_D(const QGeoMapGeometry);
    return d->mercatorToScreenPosition(mercator);
}

QGLCamera* QGeoMapGeometry::camera() const
{
    Q_D(const QGeoMapGeometry);
    return d->camera_;
}

QGLSceneNode* QGeoMapGeometry::sceneNode() const
{
    Q_D(const QGeoMapGeometry);
    return d->sceneNode_;
}

bool QGeoMapGeometry::verticalLock() const
{
    Q_D(const QGeoMapGeometry);
    return d->verticalLock_;
}

void QGeoMapGeometry::paintGL(QGLPainter *painter)
{
    Q_D(QGeoMapGeometry);
    d->paintGL(painter);
}

QGeoMapGeometryPrivate::QGeoMapGeometryPrivate(QGeoMapGeometry *geometry)
    : tileSize_(0),
      camera_(new QGLCamera()),
      sceneNode_(new QGLSceneNode()),
      scaleFactor_(10.0),
      intZoomLevel_(0),
      sideLength_(0),
      minTileX_(-1),
      minTileY_(-1),
      maxTileX_(-1),
      maxTileY_(-1),
      tileZ_(0),
      tileXWrapsBelow_(0),
      mercatorCenterX_(0.0),
      mercatorCenterY_(0.0),
      mercatorWidth_(0.0),
      mercatorHeight_(0.0),
      screenOffsetX_(0.0),
      screenOffsetY_(0.0),
      screenWidth_(0.0),
      screenHeight_(0.0),
      useVerticalLock_(false),
      verticalLock_(false),
      q_ptr(geometry) {}

QGeoMapGeometryPrivate::~QGeoMapGeometryPrivate()
{
    delete sceneNode_;
    delete camera_;
}

QDoubleVector2D QGeoMapGeometryPrivate::screenPositionToMercator(const QPointF &pos) const
{
    double x = mercatorWidth_ * (((pos.x() - screenOffsetX_) / screenWidth_) - 0.5);
    x += mercatorCenterX_;

    if (x > 1.0 * sideLength_)
        x -= 1.0 * sideLength_;
    if (x < 0.0)
        x += 1.0 * sideLength_;

    x /= 1.0 * sideLength_;

    double y = mercatorHeight_ * (((pos.y() - screenOffsetY_) / screenHeight_) - 0.5);
    y += mercatorCenterY_;
    y /= 1.0 * sideLength_;

    return QDoubleVector2D(x, y);
}

QPointF QGeoMapGeometryPrivate::mercatorToScreenPosition(const QDoubleVector2D &mercator) const
{
    double mx = sideLength_ * mercator.x();

    double lb = mercatorCenterX_ - mercatorWidth_ / 2.0;
    if (lb < 0.0)
        lb += sideLength_;
    double ub = mercatorCenterX_ + mercatorWidth_ / 2.0;
    if (sideLength_ < ub)
        ub -= sideLength_;

    double m = (mx - mercatorCenterX_) / mercatorWidth_;

    // correct for crossing dateline
    if (qFuzzyCompare(ub - lb + 1.0, 1.0) || (ub < lb) ) {
        if (mercatorCenterX_ < ub) {
            if (lb < mx) {
                m = (mx - mercatorCenterX_ - sideLength_) / mercatorWidth_;
            }
        } else if (lb < mercatorCenterX_) {
            if (mx < ub) {
                m = (mx - mercatorCenterX_ + sideLength_) / mercatorWidth_;
            }
        }
    }

    double x = screenWidth_ * (0.5 + m);
    double y = screenHeight_ * (0.5 + (sideLength_ * mercator.y() - mercatorCenterY_) / mercatorHeight_);

    return QPointF(x + screenOffsetX_, y + screenOffsetY_);
}

QGLSceneNode *QGeoMapGeometryPrivate::buildGeometry(const QGeoTileSpec &spec)
{
    int x = spec.x();

    if (x < tileXWrapsBelow_)
        x += sideLength_;

    if ((x < minTileX_)
            || (maxTileX_ < x)
            || (spec.y() < minTileY_)
            || (maxTileY_ < spec.y())
            || (spec.zoom() != tileZ_)) {
        return 0;
    }

    double edge = scaleFactor_ * tileSize_;

    QGLBuilder builder;

    double x1 = (x - minTileX_);
    double x2 = x1 + 1.0;

    double y1 = (minTileY_ - spec.y());
    double y2 = y1 - 1.0;

    x1 *= edge;
    x2 *= edge;
    y1 *= edge;
    y2 *= edge;

    QGeometryData g;

    QDoubleVector3D n = QDoubleVector3D(0, 0, 1);

    g.appendVertex(QVector3D(x1, y1, 0.0));
    g.appendNormal(n);
    g.appendTexCoord(QVector2D(0.0, 1.0));

    g.appendVertex(QVector3D(x1, y2, 0.0));
    g.appendNormal(n);
    g.appendTexCoord(QVector2D(0.0, 0.0));

    g.appendVertex(QVector3D(x2, y2, 0.0));
    g.appendNormal(n);
    g.appendTexCoord(QVector2D(1.0, 0.0));

    g.appendVertex(QVector3D(x2, y1, 0.0));
    g.appendNormal(n);
    g.appendTexCoord(QVector2D(1.0, 1.0));

    builder.addQuads(g);

    return builder.finalizedSceneNode();
}

void QGeoMapGeometryPrivate::addTile(const QGeoTileSpec &spec, QSharedPointer<QGeoTileTexture> texture)
{
    QGLSceneNode *node = nodes_.value(spec, 0);
    if (!node) {
        node = buildGeometry(spec);
        if (!node)
            return;

        QGLMaterial *mat = new QGLMaterial(node);
        mat->setTexture(texture->texture);
        node->setEffect(QGL::LitDecalTexture2D);
        node->setMaterial(mat);

        sceneNode_->addNode(node);
        nodes_.insert(spec, node);
        textures_.insert(spec, texture);
        newUploads_ << texture;

    } else {
        // TODO handle texture updates when we make node removal more efficient
        if (textures_[spec].data() != texture.data()) {
            textures_.insert(spec, texture);
            node->material()->setTexture(texture->texture);
            newUploads_ << texture;
        }
    }
}

// return true if new tiles introduced in [tiles]
void QGeoMapGeometryPrivate::setVisibleTiles(const QSet<QGeoTileSpec> &tiles)
{
    Q_Q(QGeoMapGeometry);

    // detect if new tiles introduced
    bool newTilesIntroduced = !visibleTiles_.contains(tiles);

    // work out the tile bounds for the new geometry
    setTileBounds(tiles);

    // set up the gl camera for the new geometry
    setupCamera();

    QSet<QGeoTileSpec> toRemove = visibleTiles_ - tiles;
    QSet<QGeoTileSpec> toUpdate = visibleTiles_ - toRemove;
    if (!toRemove.isEmpty())
        removeTiles(toRemove);
    if (!toUpdate.isEmpty())
        updateTiles(toUpdate);

    visibleTiles_ = tiles;
    if (newTilesIntroduced)
        emit q->newTilesVisible(visibleTiles_);
}

void QGeoMapGeometryPrivate::updateTiles(const QSet<QGeoTileSpec> &tiles)
{
    typedef QSet<QGeoTileSpec>::const_iterator iter;
    iter i = tiles.constBegin();
    iter end = tiles.constEnd();
    for (; i != end; ++i) {
        QGeoTileSpec tile = *i;
        QGLSceneNode *node = nodes_.value(tile, 0);

        if (node) {
            sceneNode_->removeNode(node);
            // TODO: re-use more of the geometry calculations?
            QGLSceneNode *newNode = buildGeometry(tile);

            if (newNode) {
                QGLMaterial *mat = new QGLMaterial(newNode);
                mat->setTexture(textures_[tile]->texture);
                newNode->setEffect(QGL::LitDecalTexture2D);
                newNode->setMaterial(mat);
                sceneNode_->addNode(newNode);
                nodes_.insert(tile, newNode);
            } else {
                nodes_.remove(tile);
                textures_.remove(tile);
            }
            delete node;
        }
    }
}

void QGeoMapGeometryPrivate::removeTiles(const QSet<QGeoTileSpec> &oldTiles)
{
    typedef QSet<QGeoTileSpec>::const_iterator iter;
    iter i = oldTiles.constBegin();
    iter end = oldTiles.constEnd();

    for (; i != end; ++i) {
        QGeoTileSpec tile = *i;
        QGLSceneNode *node = nodes_.value(tile, 0);
        if (node) {
            // TODO protect with mutex?
            sceneNode_->removeNode(node);
            nodes_.remove(tile);
            textures_.remove(tile);
            delete node;
        }
    }
}

void QGeoMapGeometryPrivate::setTileBounds(const QSet<QGeoTileSpec> &tiles)
{
    if (tiles.isEmpty()) {
        minTileX_ = -1;
        minTileY_ = -1;
        maxTileX_ = -1;
        maxTileY_ = -1;
        tileZ_ = -1;
        return;
    }

    typedef QSet<QGeoTileSpec>::const_iterator iter;
    iter i = tiles.constBegin();
    iter end = tiles.constEnd();

    tileZ_ = i->zoom();

    bool hasFarLeft = false;
    bool hasFarRight = false;
    bool hasMidLeft = false;
    bool hasMidRight = false;

    for (; i != end; ++i) {
        int x = (*i).x();
        if (x == 0)
            hasFarLeft = true;
        else if (x == (sideLength_ - 1))
            hasFarRight = true;
        else if (x == ((sideLength_ / 2) - 1)) {
            hasMidLeft = true;
        } else if (x == (sideLength_ / 2)) {
            hasMidRight = true;
        }
    }

    tileXWrapsBelow_ = 0;

    if (hasFarLeft && hasFarRight) {
        if (!hasMidRight) {
            tileXWrapsBelow_ = sideLength_ / 2;
        } else if (!hasMidLeft) {
            tileXWrapsBelow_ = (sideLength_ / 2) - 1;
        }
    }

    i = tiles.constBegin();

    QGeoTileSpec tile = *i;

    int x = tile.x();
    if (tile.x() < tileXWrapsBelow_)
        x += sideLength_;

    minTileX_ = x;
    maxTileX_ = x;
    minTileY_ = tile.y();
    maxTileY_ = tile.y();

    ++i;

    for (; i != end; ++i) {
        tile = *i;
        int x = tile.x();
        if (tile.x() < tileXWrapsBelow_)
            x += sideLength_;

        minTileX_ = qMin(minTileX_, x);
        maxTileX_ = qMax(maxTileX_, x);
        minTileY_ = qMin(minTileY_, tile.y());
        maxTileY_ = qMax(maxTileY_, tile.y());
    }
}

void QGeoMapGeometryPrivate::setupCamera()
{
    double f = 1.0 * qMin(screenSize_.width(), screenSize_.height());

    double z = pow(2.0, cameraData_.zoomLevel() - intZoomLevel_);

    double altitude = scaleFactor_ * f / (4.0 * z);

    double aspectRatio = 1.0 * screenSize_.width() / screenSize_.height();

    double a = f / (z * tileSize_);

    if (aspectRatio > 1.0) {
        mercatorHeight_ = a;
        mercatorWidth_ = a * aspectRatio;
    } else {
        mercatorWidth_ = a;
        mercatorHeight_ = a / aspectRatio;
    }

    // calculate center

    double edge = scaleFactor_ * tileSize_;

    QDoubleVector3D center = (sideLength_ * QGeoProjection::coordToMercator(cameraData_.center()));

    if (center.x() < tileXWrapsBelow_)
        center.setX(center.x() + 1.0 * sideLength_);

    mercatorCenterX_ = center.x();
    mercatorCenterY_ = center.y();

    center.setX(center.x() - 1.0 * minTileX_);
    center.setY(1.0 * minTileY_ - center.y());

    // letter box vertically
    if (useVerticalLock_ && (mercatorHeight_ > 1.0 * sideLength_)) {
        center.setY(-1.0 * sideLength_ / 2.0);
        mercatorCenterY_ = sideLength_ / 2.0;
        screenOffsetY_ = screenSize_.height() * (0.5 - sideLength_ / (2 * mercatorHeight_));
        screenHeight_ = screenSize_.height() - 2 * screenOffsetY_;
        mercatorHeight_ = 1.0 * sideLength_;
        verticalLock_ = true;
    } else {
        screenOffsetY_ = 0.0;
        screenHeight_ = screenSize_.height();
        verticalLock_ = false;
    }

    if (mercatorWidth_ > 1.0 * sideLength_) {
        screenOffsetX_ = screenSize_.width() * (0.5 - (sideLength_ / (2 * mercatorWidth_)));
        screenWidth_ = screenSize_.width() - 2 * screenOffsetX_;
        mercatorWidth_ = 1.0 * sideLength_;
    } else {
        screenOffsetX_ = 0.0;
        screenWidth_ = screenSize_.width();
    }

    center *= edge;

    // calculate eye

    QDoubleVector3D eye = center;
    eye.setZ(altitude);

    // calculate up

    QDoubleVector3D view = eye - center;
    QDoubleVector3D side = QDoubleVector3D::normal(view, QDoubleVector3D(0.0, 1.0, 0.0));
    QDoubleVector3D up = QDoubleVector3D::normal(side, view);

    // old bearing, tilt and roll code
    //    QMatrix4x4 mBearing;
    //    mBearing.rotate(-1.0 * camera.bearing(), view);
    //    up = mBearing * up;

    //    QDoubleVector3D side2 = QDoubleVector3D::normal(up, view);
    //    QMatrix4x4 mTilt;
    //    mTilt.rotate(camera.tilt(), side2);
    //    eye = (mTilt * view) + center;

    //    view = eye - center;
    //    side = QDoubleVector3D::normal(view, QDoubleVector3D(0.0, 1.0, 0.0));
    //    up = QDoubleVector3D::normal(view, side2);

    //    QMatrix4x4 mRoll;
    //    mRoll.rotate(camera.roll(), view);
    //    up = mRoll * up;

    // near plane and far plane

    double nearPlane = 0.5;
    double farPlane = 2.0 * edge;

    // TODO protect with mutex?
    // set glcamera parameters
    camera_->setCenter(center);
    camera_->setEye(eye);
    camera_->setUpVector(up);
    camera_->setNearPlane(nearPlane);
    camera_->setFarPlane(farPlane);
}

void QGeoMapGeometryPrivate::paintGL(QGLPainter *painter)
{
    // TODO protect with mutex?

    // TODO add a shortcut here for when we don't need to repeat and clip the map

    // do any pending upload/releases
    while (!newUploads_.isEmpty()) {
        newUploads_.front()->texture->bind();
        newUploads_.front()->texture->clearImage();
        newUploads_.pop_front();
    }

    glEnable(GL_SCISSOR_TEST);

    painter->setScissor(QRect(screenOffsetX_, screenOffsetY_, screenWidth_, screenHeight_));

    painter->setCamera(camera_);
    sceneNode_->draw(painter);

    QGLCamera *camera = camera_;

    bool old = camera->blockSignals(true);

    glDisable(GL_DEPTH_TEST);

    double sideLength = scaleFactor_ * tileSize_ * sideLength_;

    QDoubleVector3D c = QDoubleVector3D(camera->center());
    c.setX(c.x() + sideLength);
    camera->setCenter(c);

    QDoubleVector3D e = QDoubleVector3D(camera->eye());
    e.setX(e.x() + sideLength);
    camera->setEye(e);

    painter->setCamera(camera);
    sceneNode_->draw(painter);

    c.setX(c.x() - 2 * sideLength);
    camera->setCenter(c);
    e.setX(e.x() - 2 * sideLength);
    camera->setEye(e);

    painter->setCamera(camera);
    sceneNode_->draw(painter);

    c.setX(c.x() + sideLength);
    camera->setCenter(c);
    e.setX(e.x() + sideLength);
    camera->setEye(e);

    painter->setCamera(camera);
    sceneNode_->draw(painter);

    glEnable(GL_DEPTH_TEST);

    camera->blockSignals(old);
}

QT_END_NAMESPACE
