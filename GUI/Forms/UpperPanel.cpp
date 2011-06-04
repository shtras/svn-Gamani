#include "StdAfx.h"
#include "UpperPanel.h"
#include "Gamani.h"

UpperPanel::UpperPanel():WLayout(),minimized_(false)
{

}

UpperPanel::~UpperPanel()
{

}

void UpperPanel::init()
{
  setDimensions(0, 1, 1, 0.03);
  visible_ = true;
  speedDownButton_ = new WButton();
  speedDownButton_->setDimensions(0.01, 0, 0.03, 1);
  speedDownButton_->setLabel("<<");
  speedDownButton_->setVisible(true);
  speedDownButton_->sigClick.connect(this, &UpperPanel::speedDownButtonClick);
  addWidget(speedDownButton_);

  pauseButton_ = new WButton();
  pauseButton_->setDimensions(0.04, 0, 0.03, 1);
  pauseButton_->setLabel("||");
  pauseButton_->setVisible(true);
  pauseButton_->sigClick.connect(this, &UpperPanel::pauseButtonClick);
  addWidget(pauseButton_);

  speedUpButton_ = new WButton();
  speedUpButton_->setDimensions(0.07, 0, 0.03, 1);
  speedUpButton_->setLabel(">>");
  speedUpButton_->setVisible(true);
  speedUpButton_->sigClick.connect(this, &UpperPanel::speedUpButtonClick);
  addWidget(speedUpButton_);

  minimizeButton_ = new WButton();
  minimizeButton_->setDimensions(0, 0, 0.01, 1);
  minimizeButton_->setLabel("^");
  minimizeButton_->setVisible(true);
  minimizeButton_->sigClick.connect(this, &UpperPanel::minimizeButtonClick);
  addWidget(minimizeButton_);
}

void UpperPanel::render()
{
  WLayout::render();

}

void UpperPanel::speedUpButtonClick()
{
  Gamani::getInstance().speedUp();
}

void UpperPanel::speedDownButtonClick()
{
  Gamani::getInstance().speedDown();
}

void UpperPanel::pauseButtonClick()
{
  Gamani::getInstance().pause();
}

void UpperPanel::minimizeButtonClick()
{
  if (minimized_) {
    setDimensions(0, 1, 1, 0.03);
    speedDownButton_->setVisible(true);
    pauseButton_->setVisible(true);
    speedUpButton_->setVisible(true);
    minimizeButton_->setDimensions(0, 0, 0.01, 1);
    minimized_ = false;
  } else {
    setDimensions(0, 1, 0.01, 0.03);
    speedDownButton_->setVisible(false);
    pauseButton_->setVisible(false);
    speedUpButton_->setVisible(false);
    minimizeButton_->setDimensions(0, 0, 1, 1);
    minimized_ = true;
  }
}