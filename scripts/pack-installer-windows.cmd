call pnpm install
call pnpm run conan-install-release
call pnpm run native[windows]-reconfigure-release
call pnpm run native-build
call pnpm run pack-installer
