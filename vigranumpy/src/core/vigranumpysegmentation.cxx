/************************************************************************/
/*                                                                      */
/*                 Copyright 2009 by Ullrich Koethe                     */
/*                                                                      */
/*    This file is part of the VIGRA computer vision library.           */
/*    The VIGRA Website is                                              */
/*        http://kogs-www.informatik.uni-hamburg.de/~koethe/vigra/      */
/*    Please direct questions, bug reports, and contributions to        */
/*        ullrich.koethe@iwr.uni-heidelberg.de    or                    */
/*        vigra@informatik.uni-hamburg.de                               */
/*                                                                      */
/*    Permission is hereby granted, free of charge, to any person       */
/*    obtaining a copy of this software and associated documentation    */
/*    files (the "Software"), to deal in the Software without           */
/*    restriction, including without limitation the rights to use,      */
/*    copy, modify, merge, publish, distribute, sublicense, and/or      */
/*    sell copies of the Software, and to permit persons to whom the    */
/*    Software is furnished to do so, subject to the following          */
/*    conditions:                                                       */
/*                                                                      */
/*    The above copyright notice and this permission notice shall be    */
/*    included in all copies or substantial portions of the             */
/*    Software.                                                         */
/*                                                                      */
/*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND    */
/*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   */
/*    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          */
/*    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT       */
/*    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,      */
/*    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      */
/*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR     */
/*    OTHER DEALINGS IN THE SOFTWARE.                                   */
/*                                                                      */
/************************************************************************/

#define PY_ARRAY_UNIQUE_SYMBOL vigranumpysegmentation_PyArray_API
//#define NO_IMPORT_ARRAY
#include <vigra/numpy_array.hxx>
#include <vigra/numpy_array_converters.hxx>
#include <vigra/localminmax.hxx>
#include <vigra/labelimage.hxx>
#include <vigra/watersheds.hxx>
#include <vigra/seededregiongrowing.hxx>
#include <vigra/labelvolume.hxx>
#include <vigra/watersheds3d.hxx>
#include <vigra/seededregiongrowing3d.hxx>

#include <cmath>

namespace python = boost::python;

namespace vigra
{

template < class PixelType >
python::tuple 
pythonWatersheds2D(NumpyArray<2, Singleband<PixelType> > image,
                   int neighborhood = 8,
                   NumpyArray<2, Singleband<npy_int32> > res=python::object())
{
    vigra_precondition(neighborhood == 4 || neighborhood == 8,
       "watersheds2D(image, neighborhood): neighborhood must be 4 or 8.");
    res.reshapeIfEmpty(MultiArrayShape<2>::type(image.shape(0), image.shape(1)), "Watersheds(): Output array has wrong shape.");
    unsigned int maxRegionLabel;
    switch (neighborhood)
    {
        case 4:
        {
            maxRegionLabel = watersheds(srcImageRange(image), destImage(res),
                                        FourNeighborCode());
            break;
        }
        case 8:
        {
            maxRegionLabel = watersheds(srcImageRange(image), destImage(res),
                                        EightNeighborCode());
            break;
        }
    }
    return python::make_tuple(res, maxRegionLabel);
}

VIGRA_PYTHON_MULTITYPE_FUNCTOR(pywatersheds2D, pythonWatersheds2D)


template < class PixelType,  typename LabelType >
NumpyAnyArray pythonSeededRegionGrowing2D(NumpyArray<2, Singleband<PixelType> > image,
                                    SRGType srgType=CompleteGrow, 
                                    NumpyArray<2, Singleband<LabelType> > res=python::object())
{
    res.reshapeIfEmpty(MultiArrayShape<2>::type(image.shape(0), image.shape(1)), "seededRegionGrowing2D(): Output array has wrong shape.");
    //NumpyArray<2, Singleband<LabelType> > labels(image.shape());
    extendedLocalMinima(srcImageRange(image), destImage(res));
    LabelType max_region_label =
        labelImageWithBackground(srcImageRange(res),
        destImage(res), false, 0);
    ArrayOfRegionStatistics< SeedRgDirectValueFunctor< PixelType > >
      stats(detail::RequiresExplicitCast<unsigned int>::cast(max_region_label));
    seededRegionGrowing(srcImageRange(image), srcImage(res),
        destImage(res), stats, srgType);
    return res;
}



template < class PixelType, typename LabelType >
NumpyAnyArray pythonSeededRegionGrowingSeeded2D(NumpyArray<2, Singleband<PixelType> > image, 
                                          NumpyArray<2, Singleband<LabelType> > seeds,
                                          SRGType srgType=CompleteGrow,
                                          NumpyArray<2, Singleband<LabelType> > res=python::object())
{
    vigra_precondition(image.shape() == seeds.shape(),
         "seededRegionGrowing(): magnitude and seed images must have the same "
         "size.");
    res.reshapeIfEmpty(image.shape(), "seededRegionGrowingSeeded2D(): Output array has wrong shape.");
    
    FindMinMax< LabelType > minmax;
    inspectImage(srcImageRange(seeds), minmax);
 
    ArrayOfRegionStatistics< SeedRgDirectValueFunctor< PixelType > >
        stats((unsigned int) std::ceil((double)minmax.max));
    seededRegionGrowing(srcImageRange(image), srcImage(seeds),
        destImage(res), stats, srgType);
    return res;
}



template < class PixelType >
NumpyAnyArray pythonLocalMinima2D(NumpyArray<2, Singleband<PixelType> > image,
    PixelType marker = 1,
    int neighborhood = 8,
    NumpyArray<2, Singleband<PixelType> > res = python::object()
    )
{
    res.reshapeIfEmpty(MultiArrayShape<2>::type(image.shape(0), image.shape(1)), "localMinima2D(): Output array has wrong shape.");
    switch (neighborhood)
    {
        case 4:
        {
            localMinima(srcImageRange(image), destImage(res), marker,
                FourNeighborCode());
            break;
        }
        case 8:
        {
            localMinima(srcImageRange(image), destImage(res), marker,
                EightNeighborCode());
            break;
        }
    }

    return res;
}



template < class PixelType >
NumpyAnyArray pythonExtendedLocalMinima2D(NumpyArray<2, Singleband<PixelType> > image,
    PixelType marker = 1,
    int neighborhood = 8,
    NumpyArray<2, Singleband<PixelType> > res = python::object()
    )
{
    res.reshapeIfEmpty(MultiArrayShape<2>::type(image.shape(0), image.shape(1)), "localMinima2D(): Output array has wrong shape.");
    switch (neighborhood)
    {
        case 4:
        {
            extendedLocalMinima(srcImageRange(image), destImage(res),
                marker, FourNeighborCode());
            break;
        }
        case 8:
        {
            extendedLocalMinima(srcImageRange(image), destImage(res),
                marker, EightNeighborCode());
            break;
        }
    }
    return res;
}



template < class PixelType >
NumpyAnyArray pythonLocalMaxima2D(NumpyArray<2, Singleband<PixelType> > image,
    PixelType marker = 1,
    int neighborhood = 8,
    NumpyArray<2, Singleband<PixelType> > res=python::object()
    )
{
    res.reshapeIfEmpty(MultiArrayShape<2>::type(image.shape(0), image.shape(1)), "localMinima2D(): Output array has wrong shape.");
    switch (neighborhood)
    {
        case 4:
        {
            localMinima(srcImageRange(image), destImage(res), marker,
                FourNeighborCode());
            break;
        }
        case 8:
        {
            localMinima(srcImageRange(image), destImage(res), marker,
                EightNeighborCode());
            break;
        }
    }

    return res;
}



template < class PixelType >
NumpyAnyArray pythonExtendedLocalMaxima2D(NumpyArray<2, Singleband<PixelType> > image,
    PixelType marker = 1,
    int neighborhood = 8,
    NumpyArray<2, Singleband<PixelType> > res = python::object()
    )
{
    res.reshapeIfEmpty(MultiArrayShape<2>::type(image.shape(0), image.shape(1)), "ExtendedLocalMaxima2D(): Output array has wrong shape.");
    switch (neighborhood)
    {
        case 4:
        {
            extendedLocalMaxima(srcImageRange(image), destImage(res),
                marker, FourNeighborCode());
            break;
        }
        case 8:
        {
            extendedLocalMaxima(srcImageRange(image), destImage(res),
                marker, EightNeighborCode());
            break;
        }
    }
    return res;
}


template < class PixelType >
NumpyAnyArray pythonLabelImage2D(NumpyArray<2, Singleband<PixelType> > image,
    int neighborhood = 4,
    NumpyArray<2, Singleband<PixelType> > res = python::object())
{
    res.reshapeIfEmpty(MultiArrayShape<2>::type(image.shape(0), image.shape(1)), "labelImage2D(): Output array has wrong shape.");

    switch (neighborhood)
    {
        case 4:
        {
            labelImage(srcImageRange(image), destImage(res), false);
            break;
        }
        case 8:
        {
            labelImage(srcImageRange(image), destImage(res), true);
            break;
        }
    }

    return res;
}


template < class PixelType >
NumpyAnyArray pythonLabelImageWithBackground2D(NumpyArray<2, Singleband<PixelType> > image,
    int neighborhood = 4,
    PixelType background_value = 0,
    NumpyArray<2, Singleband<PixelType> > res = python::object())
{
    res.reshapeIfEmpty(image.shape(), "labelImageWithBackground2D(): Output array has wrong shape.");

    switch (neighborhood)
    {
        case 4:
        {
            labelImageWithBackground(srcImageRange(image),
                destImage(res), false, background_value);
            break;
        }
        case 8:
        {
            labelImageWithBackground(srcImageRange(image),
                destImage(res), true, background_value);
            break;
        }
    }
    return res;
}

template < class PixelType >
python::tuple 
pythonWatersheds3D(NumpyArray<3, Singleband<PixelType> > volume,
                   int neighborhood = 6,
                   NumpyArray<3, Singleband<npy_int32> > res=python::object())
{
    vigra_precondition(neighborhood == 6 || neighborhood == 26,
       "watersheds3D(volume, neighborhood): neighborhood must be 6 or 26.");
    res.reshapeIfEmpty(volume.shape(), "Watersheds(): Output array has wrong shape.");  
    unsigned int maxRegionLabel;
    switch (neighborhood)
    {
        case 6:
        {
            maxRegionLabel = watersheds3DSix(srcMultiArrayRange(volume), destMultiArray(res));
            break;
        }
        case 26:
        {
            maxRegionLabel = watersheds3DTwentySix(srcMultiArrayRange(volume), destMultiArray(res));
            break;
        }
    }
    return python::make_tuple(res, maxRegionLabel);
}
VIGRA_PYTHON_MULTITYPE_FUNCTOR(pywatersheds3D, pythonWatersheds3D)

template < class VoxelType >
NumpyAnyArray pythonSeededRegionGrowing3D(NumpyArray<3, Singleband<VoxelType> > volume,
                                    SRGType keepContours,
                                    NumpyArray<3, Singleband<npy_int32> > res=python::object())
{
    res.reshapeIfEmpty(volume.shape(), "seededRegionGrowing3D(): Output array has wrong shape.");    
    extendedLocalMinima(srcMultiArrayRange(volume), destMultiArray(res));
    int max_region_label =
        labelVolumeWithBackground(srcMultiArrayRange(res),
        destMultiArray(res), NeighborCode3DSix(), 0);
    ArrayOfRegionStatistics< SeedRgDirectValueFunctor< VoxelType > >
        stats(max_region_label);

    seededRegionGrowing3D(srcMultiArrayRange(volume),
        srcMultiArray(res), 
        destMultiArray(res), stats,
        keepContours);
    return res;
}

template < class VoxelType >
NumpyAnyArray pythonSeededRegionGrowingSeeded3D(NumpyArray<3, Singleband<VoxelType> > volume,
                                          NumpyArray<3, Singleband<npy_int32> > seeds,
                                          SRGType srgType=CompleteGrow,
                                          NumpyArray<3, Singleband<npy_int32> > res=python::object())
{
    vigra_precondition(volume.shape() == seeds.shape(),
         "seededRegionGrowingSeeded3D(): magnitude and seed volumes must have the same "
         "size.");
    res.reshapeIfEmpty(seeds.shape(), "seededRegionGrowingSeeded3D(): Output array has wrong shape.");    

    FindMinMax< npy_int32 > minmax;
    inspectMultiArray(srcMultiArrayRange(seeds), minmax);
    // create a statistics functor for region growing
    ArrayOfRegionStatistics< SeedRgDirectValueFunctor< VoxelType > >
        stats(minmax.max);
    seededRegionGrowing3D(srcMultiArrayRange(volume), srcMultiArray(seeds),
        destMultiArray(res), stats, srgType);

    return res;
}

template < class VoxelType >
NumpyAnyArray pythonLabelVolume3D(NumpyArray<3, Singleband<VoxelType> > volume, 
                            int neighborhood=6,
                            NumpyArray<3, Singleband<VoxelType> > res=python::object())
{
    
    res.reshapeIfEmpty(volume.shape(), "labelVolume3D(): Output array has wrong shape.");
    switch (neighborhood)
    {
    case 6:
        {
            labelVolume(srcMultiArrayRange(volume),
                destMultiArray(res), NeighborCode3DSix());
            break;
        }
        case 26:
        {
            labelVolume(srcMultiArrayRange(volume),
                destMultiArray(res), NeighborCode3DTwentySix());
            break;
        }
    }
    return res;
}

template < class VoxelType >
NumpyAnyArray pythonLabelVolumeWithBackground3D(NumpyArray<3, Singleband<VoxelType> > volume, 
                                          int neighborhood=6,
                                          Int32 background_value = 0,
                                          NumpyArray<3, Singleband<VoxelType> > res=python::object())
{
    res.reshapeIfEmpty(volume.shape(), "labelVolumeWithBackground3D(): Output array has wrong shape.");
    switch (neighborhood)
    {
        case 6:
        {
            labelVolumeWithBackground(srcMultiArrayRange(volume),
                destMultiArray(res), NeighborCode3DSix(),
                background_value);
            break;
        }
        case 26:
        {
            labelVolumeWithBackground(srcMultiArrayRange(volume),
                destMultiArray(res), NeighborCode3DTwentySix(),
                background_value);
            break;
        }
    }
    return res;
}

void defineSegmentation()
{
    using namespace python;
    
    enum_<vigra::SRGType>("SRGType")
        .value("CompleteGrow", vigra::CompleteGrow)
        .value("CompleteGrowContours", vigra::KeepContours)
        ;

    def("seededRegionGrowingSeeded2D",
        registerConverters(&pythonSeededRegionGrowingSeeded2D<float, npy_int32>),
        (arg("image"), 
         arg("seeds"), 
         arg("srgType")=CompleteGrow,
         arg("out")=python::object()),
        "Region Segmentation by means of Seeded Region Growing.\n"
        "\n"
        "This algorithm implements seeded region growing as described in\n"
        "R. Adams, L. Bischof: \"<em> Seeded Region Growing</em>\", IEEE Trans. on Pattern Analysis and Maschine Intelligence, vol 16, no 6, 1994, and\n"
        "Ullrich Koethe: Primary Image Segmentation, in: G. Sagerer, S. Posch, F. Kummert (eds.): Mustererkennung 1995, Proc. 17. DAGM-Symposium, Springer 1995\n"
        "\n"
        "In differnece to seededReegionGrowing2D, the have to be provided in the seeds image\n"
        "\n"
        "For details see seededRegionGrowing_ in the vigra C++ documentation.\n"
        );
/*  FIXME: int64 is unsupported by the C++ code (hard-coded int)
    def("seededRegionGrowingSeeded2D",
        registerConverters(&pythonSeededRegionGrowingSeeded2D<float, npy_uint64>),
        (arg("image"), 
         arg("seeds"), 
         arg("srgType")=CompleteGrow,
         arg("out")=python::object()));
*/    
    
    def("seededRegionGrowing2D",
        registerConverters(&pythonSeededRegionGrowing2D<float, npy_int32>),
        (arg("image"),
         arg("srgType")=CompleteGrow,
         arg("out")=python::object()),
        "Region Segmentation by means of Seeded Region Growing.\n"
        "\n"
        "This algorithm implements seeded region growing as described in\n"
        "R. Adams, L. Bischof: \"<em> Seeded Region Growing</em>\", IEEE Trans. on Pattern Analysis and Maschine Intelligence, vol 16, no 6, 1994, and\n"
        "Ullrich Koethe: Primary Image Segmentation, in: G. Sagerer, S. Posch, F. Kummert (eds.): Mustererkennung 1995, Proc. 17. DAGM-Symposium, Springer 1995\n"
        "\n"
        "In differnece to seededReegionGrowing2DSeeded, the seeds are calculated by finding local minima.\n"
        "\n"
        "For details see seededRegionGrowing_ in the vigra C++ documentation.\n"
        );
/*  FIXME: int64 is unsupported by the C++ code (hard-coded int)
    def("seededRegionGrowing2D",
        registerConverters(&pythonSeededRegionGrowing2D<float, npy_int64>),
        (arg("image"),
         arg("srgType")=CompleteGrow,
         arg("out")));
*/
    def("localMinima2D",
        registerConverters(&pythonLocalMinima2D<float>),
        (arg("image"), 
         arg("marker")=1, 
         arg("neighborhood2D") = 8,
         arg("out")=python::object()),
        "Find local minima in an image.\n\n"
        "For details see localMinima_ in the vigra C++ documentation.\n");

    def("extendedLocalMinima2D",
        registerConverters(&pythonExtendedLocalMinima2D<float>),
        (arg("image"), 
         arg("marker")=1, 
         arg("neighborhood2D") = 8,
         arg("out")=python::object()),
        "Find local minima in an image.\n\n"
        "For details see extendedLocalMinima_ in the vigra C++ documentation.\n"
        );

    def("localMaxima2D",
        registerConverters(&pythonLocalMaxima2D<float>),
        (arg("image"), 
         arg("marker")=1, 
         arg("neighborhood2D") = 8,
         arg("out")=python::object()),
        "Find local maxima in an image.\n\n"
        "For details see localMinima_ in the vigra C++ documentation.\n");

    def("extendedLocalMaxima2D",
        registerConverters(&pythonExtendedLocalMaxima2D<float>),
        (arg("image"), 
         arg("marker")=1, 
         arg("neighborhood2D") = 8,
         arg("out")=python::object()),
        "Find local maxima in an image.\n\n"
        "For details see localMinima_ in the vigra C++ documentation.\n");

    def("labelImage",
        registerConverters(&pythonLabelImage2D<float>),
        (arg("image"), 
        arg("neighborhood2D") = 4,
        arg("out")=python::object()),
        "Find the connected components of a segmented image.\n\n"
        "For detailt see labelImage_ in the vigra C++ documentation.\n");

    def("labelImageWithBackground",
        registerConverters(&pythonLabelImageWithBackground2D<float>),
        (arg("image"), 
        arg("neighborhood2D") = 4,
        arg("background_value") = 0,
        arg("out")=python::object()),
        "Find the connected components of a segmented image, excluding the background from labeling.\n\n"
        "For detailt see labelImageWithBackground_ in the vigra C++ documentation.\n");

    multidef("watersheds", pywatersheds2D< UInt8, float >(),
      (arg("image"), arg("neighborhood") = 8, arg("out")=python::object()),
         "Compute the watersheds of a 2D or 3D image by means of the union-find algorithm. "
         "'neighborhood' must be 4 or 8 for 2D images, and 6 or 26 for 3D images. "
         "The function returns a Python tuple::\n\n"
         "   (labelImage, maxRegionLabel)\n\n"
         "For details see watersheds_ and watersheds3D_ in the vigra C++ documentation.\n"
         );

    multidef("watersheds", pywatersheds3D< UInt8, float >(),
      (arg("volume"), arg("neighborhood")=6, arg("out")=python::object()));

/*
    def("seededRegionGrowing3D",
        registerConverters(&seededRegionGrowing3D<float>),
        (arg("volume"),
        arg("srgType")=CompleteGrow,
        arg("out")=python::object()));
*/
    def("seededRegionGrowingSeeded3D",
        registerConverters(&pythonSeededRegionGrowingSeeded3D<float>),
        (arg("volume"), arg("seedVolume"),
        arg("srgType")=CompleteGrow,
        arg("out")=python::object()),
        "Three-dimensional Region Segmentation by means of Seeded Region Growing.\n"
        "\n"
        "For details see seededRegionGrowing3D_ in the vigra C++ documentation.\n");


    def("labelVolume3D",
        registerConverters(&pythonLabelVolume3D<npy_int32>),
        (arg("volume"), 
        arg("neighborhood3D")=6,
        arg("out")=python::object()),
        "Find the connected components of a segmented volume.\n"
        "\n"
        "For details see labelVolume_ in the vigra C++ documentation.\n");

    def("labelVolumeWithBackground3D",
        registerConverters(&pythonLabelVolumeWithBackground3D<npy_int32>),
        (arg("volume"), 
         arg("neighborhood")=6, 
         arg("background_value")=0,
         arg("out")=python::object()),
        "Find the connected components of a segmented volume, excluding the background from labeling.\n"
        "\n"
        "For details see labelVolumeWithBackground_ in the vigra C++ documentation.\n");
}

} // namespace vigra

using namespace vigra;
using namespace boost::python;

BOOST_PYTHON_MODULE_INIT(segmentation)
{
    import_vigranumpy();
    defineSegmentation();
}