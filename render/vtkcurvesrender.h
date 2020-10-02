#pragma once
#include <gbslib/bscanalysis.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkPoints.h>
#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
#include <vtkCollectionIterator.h>
namespace gbs
{
    template<typename T,size_t dim>
    auto make_vtkPoint(const std::array<T,dim> &pt) -> std::array<double,3>
    {
         std::array<double,3> x;
        for(auto i = 0 ; i < fmax(dim,3);i++) x[i] = pt[i];
        return x;
    }
    
    template <typename Container>
    auto make_vtkPoints(const Container &pts) -> vtkSmartPointer<vtkPoints>
    {
            vtkSmartPointer<vtkPoints> points =
        vtkSmartPointer<vtkPoints>::New();
        std::for_each(pts.begin(), pts.end(), [&](const auto pt_) { points->InsertNextPoint(make_vtkPoint(pt_).data()); });
        return points;
    }
    
    GBS_EXPORT auto make_polyline_(vtkPoints *pts,double *a) -> vtkSmartPointer<vtkActor>;

    GBS_EXPORT auto make_spheres_(vtkPoints *pts,double *a) -> vtkSmartPointer<vtkAssembly>;

    GBS_EXPORT auto StippledLine(vtkSmartPointer<vtkActor> &actor,
                  int lineStipplePattern = 0xFFFF,
                  int lineStippleRepeat = 1) -> void;

    GBS_EXPORT auto scale_parts(double s,vtkAssembly *a) ->void;

    // GBS_EXPORT auto make_BSC_actor(const BSCurve<T,dim> &bsc) -> vtkSmartPointer<vtkAssembly>;

    template <typename Container>
    auto make_polyline(const Container &pts,double *a) -> vtkSmartPointer<vtkActor>
    {
        return make_polyline_(make_vtkPoints(pts),a);
    }

    template <typename Container>
    auto make_spheres(const Container &pts,double *a) -> vtkSmartPointer<vtkAssembly>
    {
        return make_spheres_(make_vtkPoints(pts),a);
    }

    template<typename T, size_t dim>
    auto make_BSC_actor(const PointArray<T,dim> &pts,const PointArray<T,dim> &poles) -> vtkSmartPointer<vtkAssembly>
    {
        auto colors = vtkSmartPointer<vtkNamedColors>::New();

        auto actor_crv = gbs::make_polyline(pts,colors->GetColor3d("Tomato").GetData());

        auto ctrl_polygon = vtkSmartPointer<vtkAssembly>::New();

        auto ctr_polygon_lines = gbs::make_polyline(poles,colors->GetColor3d("PaleTurquoise").GetData());
        auto ctr_polygon_dots = gbs::make_spheres(poles,colors->GetColor3d("Red").GetData()); 
        ctrl_polygon->AddPart( ctr_polygon_lines );
        ctrl_polygon->AddPart( ctr_polygon_dots );

        gbs::StippledLine(ctr_polygon_lines,0xAAAA, 2);
        gbs::scale_parts(0.05,ctr_polygon_dots);


        auto crv_actor = vtkSmartPointer<vtkAssembly>::New();
        crv_actor->AddPart(actor_crv);
        crv_actor->AddPart(ctrl_polygon);

        return crv_actor;
    }

    template<typename T, size_t dim>
    auto make_BSC_actor(const BSCurve<T,dim> &bsc) -> vtkSmartPointer<vtkAssembly>
    {
        auto pts = gbs::discretize(bsc,36); //TODO: improve discretization
        auto poles = bsc.poles();

        return make_BSC_actor(pts,poles);

    }

    template <typename T, size_t dim>
    auto make_BSC_actor(const BSCurveRational<T, dim> &bsc) -> vtkSmartPointer<vtkAssembly>
    {
        auto pts = gbs::discretize(bsc, 36); //TODO: improve discretization
        std::vector<std::array<T,dim+1>> p{bsc.poles()};

        PointArray<T, dim> poles;
        std::vector<T> weights;
        separate_weights(p, poles, weights);

        auto crv_actor = make_BSC_actor(pts, poles);
        auto crv_actor_parts = crv_actor->GetParts();
        auto ctrl_polygon = vtkAssembly::SafeDownCast(crv_actor_parts->GetItemAsObject(1));
        if (crv_actor)
        {
            auto sph_set = vtkAssembly::SafeDownCast(ctrl_polygon->GetParts()->GetItemAsObject(1));
            if (sph_set)
            {
                auto w = weights.begin();

                auto col = vtkPropCollection::New();
                sph_set->GetActors(col);

                auto it = col->NewIterator();
                for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
                {
                    auto actor_ = vtkActor::SafeDownCast(it->GetCurrentObject());
                    if (actor_)
                    {
                        actor_->SetScale(actor_->GetScale()[0]*(*w));
                        w++;
                    }
                }

                col->Delete();
            }
        }

        return crv_actor;
    }

} // namespace gbs