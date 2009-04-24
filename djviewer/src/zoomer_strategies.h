/* 
 * File:   zoomer_strategies.h
 * Author: alexander
 *
 */

#ifndef _ZOOMER_STRATEGIES_H
#define	_ZOOMER_STRATEGIES_H

#include "zoomer.h"

#ifdef	__cplusplus
extern "C" {
#endif

// base class for zoom strategies
class ZoomStrategy
{
public:
    // get description of zoom strategy
    virtual void GetDescription(char* buffer, unsigned int size) const = 0;

    // increase zoom
    virtual void ZoomIn() {}

    // decrease zoom
    virtual void ZoomOut() {}

    // calculate zoom
    virtual void CalculateZoom() {}

    // get zoom parameters
    const ZoomerParameters& GetZoomParameters() const
    {
        return m_Parameters;
    }
protected:
    ZoomStrategy(const ZoomerParameters& parameters) : m_Parameters(parameters) {}
    ZoomerParameters m_Parameters;
};

// do nothing
class EmptyZoomStrategy : public ZoomStrategy
{
public:
    EmptyZoomStrategy(const ZoomerParameters& parameters) : ZoomStrategy(parameters) {}

    // get description of zoom strategy
    virtual void GetDescription(char* buffer, unsigned int size) const
    {
        snprintf(buffer, size, "Dummy");
    }
};

// do zoom by whole pages
class PagesZoomStrategy : public ZoomStrategy
{
public:
    // constructor
    PagesZoomStrategy(const ZoomerParameters& parameters) : ZoomStrategy(parameters) {}

    // calculate zoom
    virtual void CalculateZoom()
    {
        if (m_Parameters.zoom != 33 || m_Parameters.zoom != 50)
        {
            m_Parameters.zoom = 50;
        }
    }

    // increase zoom
    virtual void ZoomIn()
    {
        m_Parameters.zoom = 33;
    }

    // decrease zoom
    virtual void ZoomOut()
    {
        m_Parameters.zoom = 50;
    }

    // get description of zoom strategy
    virtual void GetDescription(char* buffer, unsigned int size) const
    {
        int n;

        if (m_Parameters.orient == 0 || m_Parameters.orient == 3)
        {
            n = (m_Parameters.zoom == 33) ? 9 : 4;
        }
        else
        {
            n = (m_Parameters.zoom == 33) ? 6 : 2;
        }

        snprintf(buffer, size, "%s: %i%c %s", GetLangText(const_cast<char*>("@Preview")), n, 0x16, GetLangText(const_cast<char*>("@pages")));
    }
};

// calculate zoom to fit width of current page
class FitWidthStrategy : public ZoomStrategy
{
public:
    // constructor
    FitWidthStrategy(const ZoomerParameters& parameters) : ZoomStrategy(parameters) {}

    // calculate zoom
    virtual void CalculateZoom()
    {
        ddjvu_page_t* page = ddjvu_page_create_by_pageno(m_Parameters.doc, m_Parameters.cpage - 1);

        if (page == 0)
        {
            m_Parameters.zoom = 100;
            return;
        }

        while (!ddjvu_page_decoding_done(page));

        if (ddjvu_page_decoding_error(page))
        {
            ddjvu_page_release(page);

            m_Parameters.zoom = 100;
            return;
        }

        ddjvu_rect_t rect = {0, 0, ScreenWidth(), ScreenHeight()};
        ddjvu_format_t* fmt = ddjvu_format_create(DDJVU_FORMAT_GREY8, 0, 0);
        ddjvu_format_set_row_order(fmt, 1);
        ddjvu_format_set_y_direction(fmt, 1);

        unsigned char* data = new unsigned char[ScreenWidth() * ScreenHeight()];

        if (ddjvu_page_render(page, DDJVU_RENDER_COLOR, &rect, &rect, fmt, ScreenWidth(), reinterpret_cast<char*>(data)))
        {
            // calculate left offset
            int left = CalculateOffset(data, 0, ScreenHeight() / 4, ScreenWidth() / 4, ScreenHeight() / 2, true);

            // calculate right offset
            int right = CalculateOffset(data, 3 * ScreenWidth() / 4, ScreenHeight() / 4 - 1, ScreenWidth() / 4, ScreenHeight() / 2, false);

            // select smaller from two offsets and calculate zoom based on this value
            m_Parameters.zoom = 100 * (0.5 * ScreenWidth() / (0.5 * ScreenWidth() - (left < right ? left : right)));
        }
        else
        {
            m_Parameters.zoom = 100;
        }

        delete[] data;
        ddjvu_format_release(fmt);
        ddjvu_page_release(page);
    }

    // get description of zoom strategy
    virtual void GetDescription(char* buffer, unsigned int size) const
    {
        snprintf(buffer, size, "%s", GetLangText(const_cast<char*>("@Fit_width")));
    }
private:
    int CalculateOffset(unsigned char* data, int x, int y, int w, int h, bool startFromLeft)
    {
        if (startFromLeft)
        {
            unsigned char* p = data + x + ScreenWidth() * y;

            for (int i = x; i < x + w; ++i, p = data + i + x + ScreenWidth() * y)
            {
                for (int j = y; j < y + h; ++j, p += ScreenWidth())
                {
                    if (*p != 0xFF)
                    {
                        return i - x;
                    }
                }
            }
        }
        else
        {
            unsigned char* p = data + (ScreenWidth() - (x + w)) + ScreenWidth() * y;

            for (int i = x + w; i > x; --i, p = data + i + (ScreenWidth() - (x + w)) + ScreenWidth() * y)
            {
                for (int j = y; j < y + h; ++j, p += ScreenWidth())
                {
                    if (*p != 0xFF)
                    {
                        return (x + w) - i;
                    }
                }
            }
        }

        return h;
    }

    ddjvu_document_t* m_Document;
    int               m_Page;
};

// allow user to choose appropriate zoom
class ManualZoomStrategy : public ZoomStrategy
{
public:
    // constructor
    ManualZoomStrategy(const ZoomerParameters& parameters) : ZoomStrategy(parameters) {}

    // increase zoom
    virtual void ZoomIn()
    {
        m_Parameters.zoom += 5 - (m_Parameters.zoom % 5);
        if (m_Parameters.zoom > 195)
        {
            m_Parameters.zoom = 195;
        }
    }

    // decrease zoom
    virtual void ZoomOut()
    {
        m_Parameters.zoom -= 5;

        if (m_Parameters.zoom % 5)
        {
            m_Parameters.zoom += 5 - (m_Parameters.zoom % 5);
        }

        if (m_Parameters.zoom < 55)
        {
            m_Parameters.zoom = 55;
        }
    }

    // get description of zoom strategy
    virtual void GetDescription(char* buffer, unsigned int size) const
    {
        snprintf(buffer, size, "%s: %i%c", GetLangText(const_cast<char*>("@Normal_page")), m_Parameters.zoom, 0x16);
    }
};

// do zoom by columns
class ColumnsZoomStrategy : public ZoomStrategy
{
public:
    // constructor
    ColumnsZoomStrategy(const ZoomerParameters& parameters) : ZoomStrategy(parameters) {}

    // calculate zoom
    virtual void CalculateZoom()
    {
        if (m_Parameters.zoom < 200)
        {
            m_Parameters.zoom = 100;
        }
    }

    // increase zoom
    virtual void ZoomIn()
    {
        if (m_Parameters.zoom < 100)
        {
            m_Parameters.zoom = 100;
        }
        else
        {
            m_Parameters.zoom += 100 - (m_Parameters.zoom % 100);
        }

        if (m_Parameters.zoom > 400)
        {
            m_Parameters.zoom = 400;
        }
    }

    // decrease zoom
    virtual void ZoomOut()
    {
        m_Parameters.zoom -= 100;

        if (m_Parameters.zoom % 100)
        {
            m_Parameters.zoom += 100 - (m_Parameters.zoom % 100);
        }

        if (m_Parameters.zoom < 100)
        {
            m_Parameters.zoom = 100;
        }
    }

    // get description of zoom strategy
    virtual void GetDescription(char* buffer, unsigned int size) const
    {
        int columns = m_Parameters.zoom / 100;
        snprintf(buffer, size, "%s: %i%c", GetLangText(const_cast<char*>("@Columns")), columns < 1 ? 1 : columns, 0x16);
    }
};


#ifdef	__cplusplus
}
#endif

#endif	/* _ZOOMER_STRATEGIES_H */

