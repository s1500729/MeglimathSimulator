#pragma once
#include "Cell.h"
#include "Transform.h"

// TODO: FieldGenerator���쐬

class Field
{
private:
	Grid<Cell> _cells;

public:
	Grid<Cell> GetCells() const;

	/// <summary>
	/// �Z����h��
	/// </summary>
	/// <param name="pos">�h��Z���̍��W</param>
	/// <param name="team">�Z����h��`�[��</param>
	void PaintCell(Point pos, TeamType team);

	/// <summary>
	/// �^�C�������
	/// </summary>
	/// <param name="pos">�^�C���̍��W</param>
	void RemoveTile(Point pos);

	/// <summary>
	/// �w����W���t�B�[���h�̒��ł��邩���肷��
	/// </summary>
	/// <param name="pos">�t�B�[���h�������肷����W</param>
	/// <returns>���W���t�B�[���h���ł��邩</returns>
	bool IsInField(Point pos);

public:
	Field();

	void operator=(const Field& other);

	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	/// <param name="size">�t�B�[���h�̑傫��</param>
	Field(Size size);

	virtual ~Field();
};