#!/bin/bash
xcodebuild clean build -quiet CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO PRODUCT_BUNDLE_IDENTIFIER="com.michael.resolutionsetter" -sdk iphoneos -configuration Release
cd build/Release-iphoneos
ln -s ./ Payload
zip -r9 resolutionsetter.ipa Payload/resolutionsetter.app
mv resolutionsetter.ipa ../..
