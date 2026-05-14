ARG BASE_IMAGE
FROM ${BASE_IMAGE}

ARG DISTRO

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

RUN set -eux; \
    case "${DISTRO}" in \
        ubuntu|debian) \
            export DEBIAN_FRONTEND=noninteractive; \
            apt-get update; \
            apt-get install -y --no-install-recommends \
                build-essential ca-certificates git libsecret-1-dev \
                pkg-config qmake6 qt6-base-dev qt6-base-dev-tools \
                qt6-declarative-dev qt6-l10n-tools qt6-positioning-dev \
                qt6-tools-dev qt6-tools-dev-tools qt6-webchannel-dev \
                qt6-webengine-dev \
                libocct-foundation-dev libocct-modeling-data-dev \
                libocct-modeling-algorithms-dev libocct-visualization-dev \
                libocct-ocaf-dev libocct-data-exchange-dev; \
            rm -rf /var/lib/apt/lists/*; \
            ;; \
        fedora) \
            dnf install -y --setopt=install_weak_deps=False \
                ca-certificates gcc-c++ git make libsecret-devel \
                pkgconf-pkg-config qt6-qtbase-devel \
                qt6-qtdeclarative-devel qt6-qtpositioning-devel \
                qt6-qttools-devel qt6-linguist qt6-qtwebchannel-devel \
                qt6-qtwebengine-devel opencascade-devel; \
            dnf clean all; \
            rm -rf /var/cache/dnf; \
            ;; \
        *) \
            echo "Unsupported DISTRO: ${DISTRO}" >&2; \
            exit 1; \
            ;; \
    esac
