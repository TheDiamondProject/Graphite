//
// Created by Tom Hancocks on 20/02/2020.
//

#if !defined(GRAPHITE_QD_SURFACE)
#define GRAPHITE_QD_SURFACE

#include <memory>
#include <vector>
#include <libGraphite/quickdraw/geometry.hpp>
#include "libGraphite/quickdraw/internal/color.hpp"

namespace graphite::qd
{

    /**
     * The `graphite::qd::surface` class is an internal component of Graphite's QuickDraw implementation
     * and the component that provides image drawing functionality. This component should not be interacted
     * with directly, and should always go through the "QuickDraw" methods.
     */
    class surface
    {
    private:
        int m_width;
        int m_height;
        std::vector<graphite::qd::color> m_data;

    public:

        /**
         * Construct a new surface with the specified dimensions.
         * @param width     The width of the surface in pixels.
         * @param height    The height of the surface in pixels.
         */
        surface(int width, int height);

        /**
         * Construct a new surface with the specified dimensions and rgb data.
         * @param width     The width of the surface in pixels.
         * @param height    The height of the surface in pixels.
         * @param rgb       The rgb data of the surface.
         */
        surface(int width, int height, std::vector<graphite::qd::color> rgb);

        /**
         * Export the raw surface data.
         */
        [[nodiscard]] auto raw() const -> std::vector<uint32_t>;

        /**
         * Returns the size of the surface
         */
        [[nodiscard]] auto size() const -> qd::size;

        /**
         * Returns the color at the specified coordinate within the surface.
         * @param x         The x position in the surface
         * @param y         The y position in the surface
         * @return          The color
         *
         * @note            This method of getting colors is _slow_. Use only for single point lookup.
         */
        [[nodiscard]] auto at(int x, int y) const -> graphite::qd::color;

        /**
         * Set the color at the specified coordinate within the surface.
         * @param x         The x position in the surface
         * @param y         The y position in the surface
         * @param color     The color
         *
         * @note            This method of setting colors is _slow_. Use only for single point setting.
         */
        auto set(int x, int y, graphite::qd::color color) -> void;

        /**
         * Set the color at the specified coordinate within the surface.
         * @param offset    The absolute offset in the surface data
         * @param color     The color
         *
         * @note            This method of setting colors is _slow_. Use only for single point setting.
         */
        auto set(int offset, graphite::qd::color color) -> void;

        /**
         * Draw a line from point x0,y0 to x1,y1 using the color specified.
         * @param x0
         * @param y0
         * @param x1
         * @param y1
         * @param color
         */
        auto draw_line(int x0, int y0, int x1, int y1, graphite::qd::color color) -> void;

    };

}

#endif //GRAPHITE_SURFACE_H
