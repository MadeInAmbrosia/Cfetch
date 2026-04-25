pkgname=cafetch
pkgver=2.0.0
pkgrel=2
pkgdesc="cafetch"
arch=('x86_64')
url="https://github.com/idislikeubuntu/cafetch"
license=('MIT')
depends=('glibc')
makedepends=('gcc' 'lsb-release')
source=("cafetch.c" "README.md" "LICENSE")
sha256sums=('SKIP' 'SKIP' 'SKIP')

build() {
  gcc cafetch.c -o cafetch
}

package() {
  install -Dm755 cafetch "$pkgdir/usr/bin/cafetch"
  install -Dm644 "$srcdir/README.md" "$pkgdir/usr/share/doc/cafetch/README.md"
  install -Dm644 "$srcdir/LICENSE" "$pkgdir/usr/share/licenses/cafetch/LICENSE"
}
