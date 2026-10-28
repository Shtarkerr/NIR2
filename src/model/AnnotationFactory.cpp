#include "model/AnnotationFactory.h"

#include "model/ContourAnnotation.h"
#include "model/EllipseAnnotation.h"
#include "model/PointAnnotation.h"
#include "model/RectAnnotation.h"

std::unique_ptr<Annotation> AnnotationFactory::create(const QString &typeName)
{
    if (typeName == QLatin1String("point"))
        return std::make_unique<PointAnnotation>();
    if (typeName == QLatin1String("rect"))
        return std::make_unique<RectAnnotation>();
    if (typeName == QLatin1String("ellipse"))
        return std::make_unique<EllipseAnnotation>();
    if (typeName == QLatin1String("contour"))
        return std::make_unique<ContourAnnotation>();
    return nullptr;
}

std::unique_ptr<Annotation> AnnotationFactory::fromJson(const QJsonObject &obj)
{
    std::unique_ptr<Annotation> annotation = create(obj.value("type").toString());
    if (!annotation || !annotation->fromJson(obj))
        return nullptr;
    return annotation;
}
