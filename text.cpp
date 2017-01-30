#include "text.h"

#include <QTest>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsItem>

Text::Text(QString text)
    : ResizeableItem()
{
   m_font = QFont("Arial", 13);
   m_text = text;
   QFontMetrics m(m_font);
   setRect(0, 0, m.width(m_text), m.height());
   m_textitem = new QGraphicsSimpleTextItem(m_text, this);
   m_textitem->setFont(m_font);
   setPen(QPen(QBrush(), 0));
}

int Text::type() const
{
    return Text::Type;
}

void Text::paint( QPainter *paint, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    if (option->state & QStyle::State_Selected)
        drawHighlightSelected(paint, option);
}

void Text::setScale(qreal x, qreal y)
{
    m_xscale = x;
    m_yscale = y;
    QTransform trans;
    trans.scale(m_xscale, m_yscale);
    m_textitem->setTransform(trans);

    QFontMetrics m(m_font);
    setRect(0, 0, m.width(m_text) * x, m.height() * y);
}

qreal Text::xscale()
{
    return m_xscale;
}

qreal Text::yscale()
{
    return m_yscale;
}

QString Text::text()
{
    return m_text;
}

bool Text::sceneEventFilter(QGraphicsItem * watched, QEvent * event)
{
    ItemHandle * handle = dynamic_cast<ItemHandle *>(watched);
    if ( handle == NULL)
        return false;

    QGraphicsSceneMouseEvent * mevent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
    if ( mevent == NULL)
    {
        return false;
    }

    switch (event->type() )
    {
    case QEvent::GraphicsSceneMousePress:
    {
        handle->setMouseState(ItemHandle::kMouseDown);
        handle->mouseDownX = mevent->pos().x();
        handle->mouseDownY = mevent->pos().y();
    }
        break;

    case QEvent::GraphicsSceneMouseRelease:
        handle->setMouseState(ItemHandle::kMouseReleased);
        break;

    case QEvent::GraphicsSceneMouseMove:
        handle->setMouseState(ItemHandle::kMouseMoving );
        break;

    default:
        return false;
    }

    if ( handle->getMouseState() == ItemHandle::kMouseMoving )
    {
        qreal x = mevent->pos().x(), y = mevent->pos().y();

        int XaxisSign = 0;
        int YaxisSign = 0;
        switch( handle->getCorner() )
        {
        case 0:
        {
            XaxisSign = +1;
            YaxisSign = +1;
        }
            break;

        case 1:
        {
            XaxisSign = -1;
            YaxisSign = +1;
        }
            break;

        case 2:
        {
            XaxisSign = -1;
            YaxisSign = -1;
        }
            break;

        case 3:
        {
            XaxisSign = +1;
            YaxisSign = -1;
        }
            break;

        case 4:
        {
            YaxisSign = +1;
        }
            break;
        case 5:
        {
            XaxisSign = -1;
        }
            break;

        case 6:
        {
            YaxisSign = -1;
        }
            break;
        case 7:
        {
            XaxisSign = +1;

        }
            break;
        }

        int xMoved = handle->mouseDownX - x;
        int yMoved = handle->mouseDownY - y;

        int newWidth = rect().width() + ( XaxisSign * xMoved);
        if ( newWidth < 40 ) newWidth  = 40;

        int newHeight = rect().height() + (YaxisSign * yMoved) ;
        if ( newHeight < 40 ) newHeight = 40;

        int deltaWidth  =   newWidth - rect().width() ;
        int deltaHeight =   newHeight - rect().height() ;

        setRect(0,0,rect().width() + deltaWidth, rect().height() + deltaHeight);
        QFontMetrics m(m_font);
        m_xscale = rect().width() / m.width(m_text);
        m_yscale = rect().height() / m.height();

        QTransform trans;
        trans.scale(m_xscale, m_yscale);
        m_textitem->setTransform(trans);

        deltaWidth *= (-1);
        deltaHeight *= (-1);

        switch(handle->getCorner())
        {
            case 0:
            {
                int newXpos = this->pos().x() + deltaWidth;
                int newYpos = this->pos().y() + deltaHeight;
                this->setPos(newXpos, newYpos);
            }
            break;

            case 1:
            {
                int newYpos = this->pos().y() + deltaHeight;
                this->setPos(this->pos().x(), newYpos);
            }
            break;

            case 3:
            {
                int newXpos = this->pos().x() + deltaWidth;
                this->setPos(newXpos, this->pos().y());
            }
            break;

            case 4:
            {
                int newYpos = this->pos().y() + deltaHeight;
                this->setPos(this->pos().x(), newYpos);
            }
            break;

            case 7:
            {
                int newXpos = this->pos().x() + deltaWidth;
                this->setPos(newXpos, this->pos().y());
            }
            break;
        }

        setHandlePositions();

        this->update();
    }
    return true;
}
