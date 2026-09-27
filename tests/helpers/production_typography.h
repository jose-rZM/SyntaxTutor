#pragma once

#include "apptypography.h"

#include <QApplication>
#include <QFont>
#include <QString>

// Applies the production typography - the scaled application font and the
// resolved app.qss - for the lifetime of the object. The rest of the suite
// runs unstyled, so everything is put back on destruction.
class ProductionTypography {
  public:
    explicit ProductionTypography(int percent)
        : savedPercent_(AppTextScale::currentPercent()),
          savedFont_(QApplication::font()),
          savedStyleSheet_(qApp->styleSheet()) {
        AppTypography::systemFont() = savedFont_;
        AppTextScale::currentPercent() = percent;
        QApplication::setFont(AppTypography::applicationFont());
        qApp->setStyleSheet(AppTypography::loadStyleSheet());
    }

    ~ProductionTypography() {
        AppTextScale::currentPercent() = savedPercent_;
        qApp->setStyleSheet(savedStyleSheet_);
        QApplication::setFont(savedFont_);
    }

    ProductionTypography(const ProductionTypography&)            = delete;
    ProductionTypography& operator=(const ProductionTypography&) = delete;

  private:
    int     savedPercent_;
    QFont   savedFont_;
    QString savedStyleSheet_;
};
