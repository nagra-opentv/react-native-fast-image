
/**
 * This code was generated by [react-native-codegen](https://www.npmjs.com/package/react-native-codegen).
 *
 * Do not edit this file as changes may cause incorrect behavior and will be lost
 * once the code is regenerated.
 *
 * @generated by codegen project: GeneratePropsCpp.js
 */

#include <react/renderer/components/rnfastimage/Props.h>
#include <react/renderer/core/propsConversions.h>

namespace facebook {
namespace react {

FastImageViewProps::FastImageViewProps(
    const FastImageViewProps &sourceProps,
    const RawProps &rawProps): ViewProps(sourceProps, rawProps),

    source(convertRawProp(rawProps, "source", sourceProps.source, {})),
    defaultSource(convertRawProp(rawProps, "defaultSource", sourceProps.defaultSource, {})),
    resizeMode(convertRawProp(rawProps, "resizeMode", sourceProps.resizeMode, {FastImageViewResizeMode::Cover})),
    tintColor(convertRawProp(rawProps, "tintColor", sourceProps.tintColor, {}))
    {}

} // namespace react
} // namespace facebook
