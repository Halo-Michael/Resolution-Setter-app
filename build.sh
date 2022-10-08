#!/bin/bash
xcodebuild clean build -quiet CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO PRODUCT_BUNDLE_IDENTIFIER="com.michael.resolutionsetter" -sdk iphoneos -configuration Release
ldid -Sresolutionsetter/resolutionsetter.entitlements -Icom.michael.resolutionsetter build/Release-iphoneos/resolutionsetter.app/resolutionsetter
cd build/Release-iphoneos
ln -s ./ Payload
zip -r9 resolutionsetter.ipa Payload/resolutionsetter.app
mv resolutionsetter.ipa ../..
